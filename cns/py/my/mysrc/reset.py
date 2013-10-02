


# M - pre, N - post

def stdp_call_post(spikes, N_in, N_out, _alreadyPotentiated, _synW, _gmax, A_pre, mu, w_out):
    nspikes = size(spikes)
    code = '''
    for(int si=0;si<nspikes;si++)
    {
        int i = spikes(si);
        for(int j=0;j<N_in;j++)
        {
            double wnew;
            wnew = _synW(j,i)+_gmax(i)*w_out;
            if(wnew<0) wnew = 0.0;
            if(!_alreadyPotentiated(j,i))
            {
                if(mu==0) { /* additive. requires hard bound */
                    wnew += _gmax(i)*A_pre(j);
                    if(wnew>_gmax(i)) wnew = _gmax(i);
                }
                else { /* soft bound */
                    wnew += _gmax(i)*A_pre(j)*exp(mu*log(1-wnew/_gmax(i)));
                }
                _alreadyPotentiated(j,i) = true;
            }
            _synW(j,i) = wnew;
        }
    }
    '''
    weave.inline(code,
            ['spikes', 'nspikes', 'N', '_alreadyPotentiated', '_synW', '_gmax', 'A_pre', 'mu','w_out'],
            compiler='gcc',
            type_converters=weave.converters.blitz,
            extra_compile_args=['-O3'])



def neurons_reset(P,spikes):
    if size(spikes):
        if not poissonOutput:
            P.v_[spikes]=vr # reset pot
        if nearestSpike:
            nspikes = size(spikes)
            A_pre = mirror.A_pre_

        stdp_call_post(spikes, N,

        P.A_post_[spikes]=a_post[spikes] # reset A_post (~start a timer for future LTD)
        _alreadyDepressed[:,spikes] = False
            

