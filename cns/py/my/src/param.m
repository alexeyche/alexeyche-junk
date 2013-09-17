global PARAM

PARAM.randState = 0; % Random generator seed. 
PARAM.T = 1000; % Total duration (s)
PARAM.patternDuration = 50e-3; % Patter duration L (s)
PARAM.meanFreq = 20; % Mean firing rate r_0 (Hz)
PARAM.nAfferent = 1000; % Number of afferents N
PARAM.nAfferentInPattern = round(2^-1*PARAM.nAfferent); % Number of pattern afferents N_0
PARAM.patternFreq = 1.5 * PARAM.patternDuration;  % Pattern presentation frequency f
PARAM.deletion = 0;%1-1/sqrt(2); % Deletion probability (=1-p)

% type of pattern
PARAM.ACADEMIC = 0; % Not used anymore
PARAM.POISSON = 1; % Baseline simulation
PARAM.N1N2 = 2; % Bimodal patterns (see Fig. 4)
PARAM.FIG = 3; % Not used anymore
PARAM.PSTH = 4; % Not used anymore
PARAM.pattern_type = PARAM.POISSON;

% used peaked instantaneous rates
PARAM.peakedRate = true; % True for model R, false for model S
PARAM.rCoef = false; % Use global enveloppe (see Fig. 7)
PARAM.dt = .5e-3; % Time step
PARAM.r0 = 0; % Spontaneous activity
PARAM.speak0 = 10e-3; % Sigma of Gaussian rate peaks
PARAM.speak1 = 0*PARAM.speak0; % Sigma of Gaussian rate peak variability
PARAM.rpeak0 = 1; % Height of Gaussian rate peaks
PARAM.rpeak1 = 0*PARAM.rpeak0;% Height of Gaussian rate peak variability

% random positions of pattern (avoiding consecutive ones)
rand('state',PARAM.randState)
PARAM.posPattern = [];
skip = false;
for p = 1:floor( PARAM.T / PARAM.patternDuration )
    if skip
        skip = false;
    else
        if rand<1/(1/PARAM.patternFreq-1)
            PARAM.posPattern = [PARAM.posPattern p];
            skip = true; % skip next
        end
    end
end
patternPeriod = zeros(length(PARAM.posPattern),2);
for p=1:length(PARAM.posPattern)
    patternPeriod(p,:) = ( PARAM.posPattern(p) + [-1  0] ) * PARAM.patternDuration;
end
save('-V7', ['../data/patternPeriod.' sprintf('%03d',PARAM.randState) '.mat'], 'patternPeriod');
