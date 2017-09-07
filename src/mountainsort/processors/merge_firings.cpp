#include "merge_firings.h"
#include "hungarian.h"
#include "get_sort_indices.h"

#include <diskreadmda.h>

struct MFEvent {
    int chan;
    double time;
    int label;
};
struct MFMergeEvent {
    int chan;
    double time;
    int label1;
    int label2;
};

void sort_events_by_time(QList<MFEvent>& events);
void sort_events_by_time(QList<MFMergeEvent>& events);
int compute_max_label(const QList<MFEvent>& events);

bool merge_firings(QString firings1_path, QString firings2_path, QString firings_merged_path, QString confusion_matrix_path, QString optimal_label_map_path, int max_matching_offset)
{
    // The events from firings1
    QList<MFEvent> events1;
    {
        DiskReadMda F1(firings1_path);
        for (int i = 0; i < F1.N2(); i++) {
            MFEvent evt;
            evt.chan = F1.value(0, i);
            evt.time = F1.value(1, i);
            evt.label = F1.value(2, i);
            events1 << evt;
        }
    }
    // The events from firings2
    QList<MFEvent> events2;
    {
        DiskReadMda F2(firings2_path);
        for (int i = 0; i < F2.N2(); i++) {
            MFEvent evt;
            evt.chan = F2.value(0, i);
            evt.time = F2.value(1, i);
            evt.label = F2.value(2, i);
            events2 << evt;
        }
    }

    // Sort the events by time
    sort_events_by_time(events1);
    sort_events_by_time(events2);

    int K1 = compute_max_label(events1);
    int K2 = compute_max_label(events2);

    //count up every pair that satisfies max_matching_offset -- and I mean EVERY PAIR
    int total_counts[K1 + 1][K2 + 1];
    {
        for (int k2 = 0; k2 < K2 + 1; k2++) {
            for (int k1 = 0; k1 < K1 + 1; k1++) {
                total_counts[k1][k2] = 0;
            }
        }

        int i2 = 0;
        for (int i1 = 0; i1 < events1.count(); i1++) {
            if (events1[i1].label > 0) {
                double t1 = events1[i1].time;
                while ((i2 < events2.count()) && (events2[i2].time < t1 - max_matching_offset))
                    i2++;
                int old_i2 = i2;
                while ((i2 < events2.count()) && (events2[i2].time <= t1 + max_matching_offset)) {
                    if (events2[i2].label > 0) {
                        total_counts[events1[i1].label][events2[i2].label]++;
                    }
                    i2++;
                }
                i2 = old_i2;
            }
        }
    }

    // Count up just the events in firings2, this will be used to normalize the match score
    int total_events2_counts[K2 + 1];
    for (int k2 = 0; k2 < K2 + 1; k2++) {
        total_events2_counts[k2] = 0;
    }
    for (int i2 = 0; i2 < events2.count(); i2++) {
        if (events2[i2].label > 0) {
            total_events2_counts[events2[i2].label]++;
        }
    }

    // Create the merged events
    QList<MFMergeEvent> events3;
    {
        // This will be the index in firings2
        int i2 = 0;
        // Loop through all of the events in firings1
        for (int i1 = 0; i1 < events1.count(); i1++) {
            if (events1[i1].label > 0) { // only condider it if it has a label
                double t1 = events1[i1].time; // this is the timepoint for event 1

                //increase i2 until it reaches the left-constraint
                while ((i2 < events2.count()) && (events2[i2].time < t1 - max_matching_offset))
                    i2++;
                int old_i2 = i2; //save this index for later so we can return to this spot for the next event

                double best_match_score = 0;
                int best_i2 = -1;
                //move through the events in firings2 until we pass the right-constraint
                while ((i2 < events2.count()) && (events2[i2].time <= t1 + max_matching_offset)) {
                    if (events2[i2].label > 0) { //only consider it if it has a label
                        //total_counts[events1[i1].label][events2[i2].label]++; //oh boy this was a mistake. Commented out on 10/13/16 by jfm
                        int tmp = total_counts[events1[i1].label][events2[i2].label]; //this is the count
                        double denom = total_events2_counts[events2[i2].label];
                        if (denom < 50)
                            denom = 50; //let's make sure it is something reasonable!
                        double match_score = tmp / denom; //normalize the match score -- which is not what we did in the past. changed on 10/13/16 by jfm
                        if (match_score > best_match_score) {
                            best_match_score = match_score;
                            best_i2 = i2;
                        }
                    }
                    i2++;
                }
                if (best_i2 >= 0) {
                    // Create the merge event if there is something that matched
                    MFMergeEvent E;
                    E.chan = events1[i1].chan;
                    E.time = events1[i1].time;
                    E.label1 = events1[i1].label;
                    E.label2 = events2[best_i2].label;
                    events3 << E;
                    events1[i1].time = -1;
                    events1[i1].label = -1; //set to -1 , see below
                    events2[best_i2].time = -1;
                    events2[best_i2].label = -1;
                }
                i2 = old_i2; // go back so we are ready to handle the next event
            }
        }
    }

    //Now take care of all the events that did not get handled earlier ... they are unclassified
    for (int i1 = 0; i1 < events1.count(); i1++) {
        if (events1[i1].label > 0) { //this is how we check to see if it was not handled
            MFMergeEvent E;
            E.chan = events1[i1].chan;
            E.time = events1[i1].time;
            E.label1 = events1[i1].label;
            E.label2 = 0;
            events3 << E;
        }
    }
    for (int i2 = 0; i2 < events2.count(); i2++) {
        if (events2[i2].label > 0) {
            MFMergeEvent E;
            E.chan = events2[i2].chan;
            E.time = events2[i2].time;
            E.label1 = 0;
            E.label2 = events2[i2].label;
            ;
            events3 << E;
        }
    }

    // Sort the merged events by time
    sort_events_by_time(events3);
    int L = events3.count();

    // Create the confusion matrix
    Mda confusion_matrix(K1 + 1, K2 + 1);
    {
        for (int i = 0; i < L; i++) {
            int a = events3[i].label1 - 1;
            int b = events3[i].label2 - 1;
            if (a < 0)
                a = K1;
            if (b < 0)
                b = K2;
            confusion_matrix.setValue(confusion_matrix.value(a, b) + 1, a, b);
        }
    }
    confusion_matrix.write64(confusion_matrix_path);

    // Get the optimal label map
    Mda optimal_label_map(K1, 1);
    if (K1 > 0) {
        int assignment[K1];
        Mda matrix(K1, K2);
        for (int i = 0; i < K1; i++) {
            for (int j = 0; j < K2; j++) {
                matrix.setValue((-1) * confusion_matrix.value(i, j), i, j);
            }
        }
        double cost;
        hungarian(assignment, &cost, matrix.dataPtr(), K1, K2);
        for (int i = 0; i < K1; i++) {
            optimal_label_map.setValue(assignment[i] + 1, i);
        }
    }
    optimal_label_map.write64(optimal_label_map_path);

    // Create the merged firings file
    Mda firings_merged(4, L);
    {
        for (int i = 0; i < L; i++) {
            MFMergeEvent* E = &events3[i];
            firings_merged.setValue(E->chan, 0, i);
            firings_merged.setValue(E->time, 1, i);
            firings_merged.setValue(E->label1, 2, i);
            firings_merged.setValue(E->label2, 3, i);
        }
    }
    firings_merged.write64(firings_merged_path);

    return true;
}

void sort_events_by_time(QList<MFEvent>& events)
{
    QVector<double> times(events.count());
    for (int i = 0; i < events.count(); i++) {
        times[i] = events[i].time;
    }
    QList<int> inds = get_sort_indices(times);
    QList<MFEvent> ret;
    for (int i = 0; i < inds.count(); i++) {
        ret << events[inds[i]];
    }
    events = ret;
}

void sort_events_by_time(QList<MFMergeEvent>& events)
{
    QVector<double> times(events.count());
    for (int i = 0; i < events.count(); i++) {
        times[i] = events[i].time;
    }
    QList<int> inds = get_sort_indices(times);
    QList<MFMergeEvent> ret;
    for (int i = 0; i < inds.count(); i++) {
        ret << events[inds[i]];
    }
    events = ret;
}

int compute_max_label(const QList<MFEvent>& events)
{
    int ret = 0;
    for (int i = 0; i < events.count(); i++) {
        if (events[i].label > ret)
            ret = events[i].label;
    }
    return ret;
}
