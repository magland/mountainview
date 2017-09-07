/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/27/2016
*******************************************************/
#ifndef ISOCUT4_H
#define ISOCUT4_H

/*
 * MCWRAP [ dipscore[1,1], cutpoint[1,1] ] = isocut4(samples[1,N])
 * SET_INPUT N = size(samples,2)
 * SOURCES isocut4.cpp jisotonic.cpp
 * HEADERS isocut4.h
 */
bool isocut4(int N, double* dipscore, double* cutpoint, double* samples);

#endif // ISOCUT4_H
