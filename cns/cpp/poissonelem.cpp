

void PoissonElem::PoissonElem(double herz_v) : herz(herz_v), Iout(0), acc(0) { }



void PoissonElem::computeMe(double dt) { 
   if(acc >= herz) {
        if(acc >= (herz + LONG_S)) {
            Iout=0;
            acc=0;
            return;
        } 

        Iout=I_OUT;
    }                
    acc+=dt
}

