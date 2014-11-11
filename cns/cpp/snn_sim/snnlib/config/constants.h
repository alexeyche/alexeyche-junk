#pragma once


struct IaFLayerC {
    double tau_refr;
    double amp_refr;
    double u_rest;
};

struct Constants {
    IaFLayerC iaf_c;
};

Constants parseConstants(const string &filename);

