#pragma once

struct AUCDatum {
    unsigned int ground_truth; //binary label, must be 0 or 1
    float prediction;
};

double CalculateAUC(AUCDatum* pFI, unsigned int _count);
