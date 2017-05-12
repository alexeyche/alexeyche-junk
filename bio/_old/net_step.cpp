#include <iostream>
#include <cmath>

using ui32 = unsigned int;
using TFloat = float;

struct TMatrix {
    TMatrix(TFloat* data, ui32 nrows, ui32 ncols)
        : Data(data)
        , Nrows(nrows) 
        , Ncols(ncols)
        
    {
    }
    
    TFloat& operator()(ui32 i, ui32 j) {
        return Data[i * Ncols + j];
    }

    ui32 Ncols;
    ui32 Nrows;
    TFloat* Data;
};


struct TVector {
    TVector(TFloat* data, ui32 n)
        : Data(data)
        , N(n) 
        
    {
    }
    
    TFloat& operator()(ui32 i) {
        return Data[i];
    }

    ui32 N;
    TFloat* Data;
};


extern "C" {
    
    double act(double x) {
        return 1.0/(1.0 + exp(-x));
    }
    
    double poisson(double r, double dt) {
        double U = (double)rand() / (double)RAND_MAX;
        return (dt * r) > U;
    }

    void net_step(
        ui32 ti,
        TFloat dt,
        bool target,
        TFloat gL,
        TFloat gB,
        TFloat gA,
        TFloat gD,
        TFloat int_time,
        TFloat batch_to_listen,
        TFloat lambda_max,
        TFloat tau_l,
        TFloat tau_s,
        TFloat koeff_epsp,
        TFloat epsp_bias,
        TFloat Ee,
        TFloat Ei,

        TFloat* _x, ui32 x_nrows, ui32 x_ncols,
        TFloat* _hidden_C, ui32 hidden_C_nrows, ui32 hidden_C_ncols,
        TFloat* _hidden_Cm, ui32 hidden_Cm_nrows, ui32 hidden_Cm_ncols,
        TFloat* _hidden_Am, ui32 hidden_Am_nrows, ui32 hidden_Am_ncols,
        TFloat* _hidden_Crate, ui32 hidden_Crate_nrows, ui32 hidden_Crate_ncols,
        TFloat* _hidden_Cact, ui32 hidden_Cact_nrows, ui32 hidden_Cact_ncols,
        TFloat* _hidden_Cact_aux, ui32 hidden_Cact_aux_nrows, ui32 hidden_Cact_aux_ncols,
        TFloat* _hidden_Bm, ui32 hidden_Bm_nrows, ui32 hidden_Bm_ncols,
        TFloat* _hidden_W, ui32 hidden_W_nrows, ui32 hidden_W_ncols,
        TFloat* _hidden_Y, ui32 hidden_Y_nrows, ui32 hidden_Y_ncols,
        TFloat* _hidden_Chist, ui32 hidden_Chist_nrows, ui32 hidden_Chist_ncols,
        TFloat* _hidden_Crate_hist, ui32 hidden_Crate_hist_nrows, ui32 hidden_Crate_hist_ncols,
        TFloat* _hidden_Cspikes_hist, ui32 hidden_Cspikes_hist_nrows, ui32 hidden_Cspikes_hist_ncols,
        TFloat* _hidden_Cact_hist, ui32 hidden_Cact_hist_nrows, ui32 hidden_Cact_hist_ncols,
        TFloat* _output_U, ui32 output_U_nrows, ui32 output_U_ncols,
        TFloat* _output_Um, ui32 output_Um_nrows, ui32 output_Um_ncols,
        TFloat* _output_Uact, ui32 output_Uact_nrows, ui32 output_Uact_ncols,
        TFloat* _output_Uact_aux, ui32 output_Uact_aux_nrows, ui32 output_Uact_aux_ncols,
        TFloat* _output_Urate, ui32 output_Urate_nrows, ui32 output_Urate_ncols,
        TFloat* _output_W, ui32 output_W_nrows, ui32 output_W_ncols,
        TFloat* _output_Ihist, ui32 output_Ihist_nrows, ui32 output_Ihist_ncols,
        TFloat* _output_Uhist, ui32 output_Uhist_nrows, ui32 output_Uhist_ncols,
        TFloat* _output_Urate_hist, ui32 output_Urate_hist_nrows, ui32 output_Urate_hist_ncols,
        TFloat* _gE, ui32 gE_nrows, ui32 gE_ncols,
        TFloat* _gI, ui32 gI_nrows, ui32 gI_ncols,

        TFloat* _hidden_b, ui32 hidden_b_n,
        TFloat* _output_b, ui32 output_b_n)
    {
        TMatrix x(_x, x_nrows, x_ncols);
        TMatrix hidden_C(_hidden_C, hidden_C_nrows, hidden_C_ncols);
        TMatrix hidden_Cm(_hidden_Cm, hidden_Cm_nrows, hidden_Cm_ncols);
        TMatrix hidden_Am(_hidden_Am, hidden_Am_nrows, hidden_Am_ncols);
        TMatrix hidden_Crate(_hidden_Crate, hidden_Crate_nrows, hidden_Crate_ncols);
        TMatrix hidden_Cact(_hidden_Cact, hidden_Cact_nrows, hidden_Cact_ncols);
        TMatrix hidden_Cact_aux(_hidden_Cact_aux, hidden_Cact_aux_nrows, hidden_Cact_aux_ncols);
        TMatrix hidden_Bm(_hidden_Bm, hidden_Bm_nrows, hidden_Bm_ncols);
        TMatrix hidden_W(_hidden_W, hidden_W_nrows, hidden_W_ncols);
        TMatrix hidden_Y(_hidden_Y, hidden_Y_nrows, hidden_Y_ncols);
        
        TMatrix hidden_Chist(_hidden_Chist, hidden_Chist_nrows, hidden_Chist_ncols);
        TMatrix hidden_Crate_hist(_hidden_Crate_hist, hidden_Crate_hist_nrows, hidden_Crate_hist_ncols);
        TMatrix hidden_Cspikes_hist(_hidden_Cspikes_hist, hidden_Cspikes_hist_nrows, hidden_Cspikes_hist_ncols);
        TMatrix hidden_Cact_hist(_hidden_Cact_hist, hidden_Cact_hist_nrows, hidden_Cact_hist_ncols);
        
        TMatrix output_U(_output_U, output_U_nrows, output_U_ncols);
        TMatrix output_Um(_output_Um, output_Um_nrows, output_Um_ncols);
        TMatrix output_Uact(_output_Uact, output_Uact_nrows, output_Uact_ncols);
        TMatrix output_Uact_aux(_output_Uact_aux, output_Uact_aux_nrows, output_Uact_aux_ncols);
        TMatrix output_Urate(_output_Urate, output_Urate_nrows, output_Urate_ncols);
        TMatrix output_W(_output_W, output_W_nrows, output_W_ncols);
        
        TMatrix output_Ihist(_output_Ihist, output_Ihist_nrows, output_Ihist_ncols);
        TMatrix output_Uhist(_output_Uhist, output_Uhist_nrows, output_Uhist_ncols);
        TMatrix output_Urate_hist(_output_Urate_hist, output_Urate_hist_nrows, output_Urate_hist_ncols);

        TVector hidden_b(_hidden_b, hidden_b_n);
        TVector output_b(_output_b, output_b_n);

        TMatrix gE(_gE, gE_nrows, gE_ncols);
        TMatrix gI(_gI, gI_nrows, gI_ncols);
        

        ui32 batch_size = hidden_C_nrows;
        ui32 hidden_size = hidden_C_ncols;
        ui32 output_size = output_U_ncols;
        ui32 input_size = x_ncols;
        
        for (int b_id=0; b_id<batch_size; b_id++) {
            for (int n_id=0; n_id<hidden_size; n_id++) {
                double input_pressure = 0.0;
                for (int inp_id=0; inp_id<input_size; inp_id++) {
                    input_pressure +=  x(b_id, inp_id) * hidden_W(inp_id, n_id) + hidden_b(n_id);
                }

                hidden_Bm(b_id, n_id) += (- hidden_Bm(b_id, n_id) + input_pressure)/int_time;
                
                double output_pressure = 0.0;
                if (target) {
                    for (int o_id=0; o_id < output_size; ++o_id) {
                        output_pressure += output_Uact(b_id, o_id) * hidden_Y(o_id, n_id);
                    }
                    
                }

                double plateau_potentials = target ? gA * (output_pressure - hidden_C(b_id, n_id)) : 0.0;
                
                hidden_C(b_id, n_id) += dt * (
                    - gL * hidden_C(b_id, n_id) + gB * (input_pressure - hidden_C(b_id, n_id)) + plateau_potentials
                );

                hidden_Crate(b_id, n_id) = lambda_max * act(hidden_C(b_id, n_id));
                double spike = poisson(hidden_Crate(b_id, n_id), dt);

                hidden_Cact_aux(b_id, n_id) += - hidden_Cact_aux(b_id, n_id)/tau_s + koeff_epsp * spike;
                hidden_Cact(b_id, n_id) += - hidden_Cact(b_id, n_id)/tau_l + epsp_bias * koeff_epsp * hidden_Cact_aux(b_id, n_id);

                if (b_id == batch_to_listen) {
                    hidden_Chist(ti, n_id) = hidden_C(b_id, n_id);
                    hidden_Crate_hist(ti, n_id) = hidden_Crate(b_id, n_id);
                    hidden_Cspikes_hist(ti, n_id) = spike;
                    hidden_Cact_hist(ti, n_id) = hidden_Cact(b_id, n_id);
                }
            }
            for (int n_id=0; n_id<output_size; n_id++) {
                double input_pressure = 0.0;
                for (int inp_id=0; inp_id<hidden_size; inp_id++) {
                    input_pressure +=  hidden_Cact(b_id, inp_id) * output_W(inp_id, n_id) + output_b(n_id);
                }

                double I = 0.0;
                if (target) {
                    I = gE(b_id, n_id) * (Ee - output_U(b_id, n_id)) + gI(b_id, n_id) * (Ei - output_U(b_id, n_id));
                }
                output_U(b_id, n_id) += dt * (
                    - gL * output_U(b_id, n_id) + gD * (input_pressure - output_U(b_id, n_id)) + I
                );

                output_Urate(b_id, n_id) = lambda_max * act(output_U(b_id, n_id));
                double spike = poisson(output_Urate(b_id, n_id), dt);

                output_Uact_aux(b_id, n_id) += - output_Uact_aux(b_id, n_id)/tau_s + koeff_epsp * spike;
                output_Uact(b_id, n_id) += - output_Uact(b_id, n_id)/tau_l + epsp_bias * koeff_epsp * output_Uact_aux(b_id, n_id);

                if (b_id == batch_to_listen) {
                    output_Ihist(ti, n_id) = I;
                    output_Uhist(ti, n_id) = output_U(b_id, n_id);
                    output_Urate_hist(ti, n_id) = output_Urate(b_id, n_id);
                }
            }
        }
    }

}
