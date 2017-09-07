/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef jisotonic_h
#define jisotonic_h

void jisotonic(int N, double* BB, double* MSE, double* AA, double* WW);
void jisotonic_updown(int N, double* out, double* in, double* weights);
void jisotonic_downup(int N, double* out, double* in, double* weights);
void jisotonic_sort(int N, double* out, const double* in);

#endif
