/* Calculate AUC using integer-based trapezoidal integration
 * Copyright (c) Bo Yang 2025
 *
 * Comments generated with assistance from Grok 3 AI.
 * 
 * I wrote this code many years ago. Recently I came across it, and asked
 * Grok 3 AI to check it for issues. I'm glad to say Grok didn't find any
 * real issues. I asked Grok to generate some documentation and here it is.
 */

//no precompiled header to keep this project simple
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "calc_auc.h"

static int cbCompareAUCDatumPrediction(const void* p0, const void* p1)
{   //callback to sort by prediction in ascending order
    const AUCDatum* pf0 = (const AUCDatum*)p0;
    const AUCDatum* pf1 = (const AUCDatum*)p1;
    if (pf0->prediction == pf1->prediction) return 0;
    return (pf0->prediction < pf1->prediction) ? -1 : +1;
}

/* Calculates AUC-ROC using trapezoidal integration with integer arithmetic

Takes an array of AUCDatum structs (ground truth must be either 0 or 1),
sort it in-place by prediction in ascending order, then integrate trapezoid
areas as we scan thru the prediction values.

To speed things up and avoid floating point inaccuracy, integration is done
using integers only. Result is normalized to double at the end.

Parameters:
  pFI: points to an array of AUCDatum structs, this will be sorted in-place
  _count: size of array pointed by pFI, must be >= 2. The area accumulation
          is guaranteed not to overflow when _count is <= 4G (2^32).

Returns:
  [0,1]: Valid AUC value
  -1: all ground truth are 0, can't compute AUC
  -2: all ground truth are 1, can't compute AUC
*/

 /** The following documentation was originally generated using Grok 3 AI and
 * then edited manually.
 *
 * @brief Integration Algorithm Description
 *
 * This implementation uses an integer-based Trapezoidal Rule to compute AUC:
 *
 * Mathematical Formula:
 * Raw_AUC = Sum [ ( TP(k) + TP(k-1) ) * deltaTN(k) ]
 * Normalized_AUC = 0.5 * Raw_AUC / (N_pos * N_neg)
 * Where:
 * - TP(k): True Positives at threshold k (integer count)
 * - deltaTN(k): change in True Negatives between thresholds (integer count)
 * - N_pos: total positives (ones)
 * - N_neg: total negatives (_count - ones)
 *
 * Algorithm Steps:
 * 1. Sorts input data by prediction in ascending order using qsort
 * 2. Initialize:
 *    - tp = ones (true positives remaining)
 *    - tn = 0 (true negatives since last threshold)
 *    - accum = 0 (area accumulator, stores twice the scaled area)
 * 3. Iterates through sorted predictions:
 *    - When prediction changes (new threshold):
 *      - Computes trapezoid area using previous and current TP counts
 *      - set tp0 = tp (true positives at last threshold)
 *      - Resets tn counter
 *    - Updates tn (x-axis) and tp (y-axis) based on ground truth
 * 4. Adds final trapezoid area
 * 5. Normalizes result to [0,1]
 *
 * Notes:
 * - Time Complexity: O(n log n) due to sorting, where n is _count
 * - ground_truth must be either 0 or 1
 * - Requires at least 2 samples with mixed labels
 * - Avoids floating-point calculation until final normalization
 * - Uses uint_fast64_t for area accumulator to prevent overflow with large
     datasets, it is guaranteed not to overflow when _count is <= 2^32
 */
double CalculateAUC(AUCDatum* pFI, unsigned int _count)
{
    assert(_count >= 2);

    unsigned int ones = 0; //count of ones in ground truth
    for (unsigned int i = 0; i < _count; i++) ones += pFI[i].ground_truth;
    if (0 == ones) {
        fprintf(stderr, "ground truth are all zeros, can't calc AUC\n");
        return -1;
    }
    if (_count == ones) {
        fprintf(stderr, "ground truth are all ones, can't calc AUC\n");
        return -2;
    }
    qsort(pFI, _count, sizeof(*pFI), cbCompareAUCDatumPrediction);

    unsigned int tn = 0; //count of true negatives since last threshold
    unsigned int tp = ones; //remaining true positives
    unsigned int tp0 = ones; //true positives at last threshold
    uint_fast64_t accum = 0; //won't overflow with 32-bit _count
    float threshold = pFI[0].prediction; //predictions <= threshold are considered zeros
    for (unsigned int i = 0; i < _count; i++) {
        if (pFI[i].prediction != threshold) { //threshold changes
            threshold = pFI[i].prediction;
            accum += (uint_fast64_t)(tp + tp0) * tn; //twice the area of trapezoid
            tp0 = tp;
            tn = 0;
        }
        tn += 1 - pFI[i].ground_truth; //increment x-axis (FPR) distance if GT 0
        tp -= pFI[i].ground_truth;
    }
    assert(tp == 0); //all 1s in ground truth should be processed now
    accum += (uint_fast64_t)tp0 * tn; //twice the area of trapezoid

    return 0.5 * (double)accum / (double)ones / (double)(_count - ones);
}
