function polychrony(exec_code)

if nargin==1
    eval(exec_code);
end;

drawbuttons;
drawfield;





%-----------------------
function initialize()
global values
global transponders traces waves circle Stop_flag

    values.sizex=600;
    values.sizey=400;
    values.speed=1;
    values.parab=0;
    values.delay = 5;   % the delay from the moment a dot is excited 
                        % to the moment it fires an excitation wave                                        
    values.tolerance=1;    
    
    transponders = {};
    stop_clear;
    
    phi = 2*pi*(-1:100)'/100;
    circle = [cos(phi), sin(phi)];
    
    values.ca = 0;
    reset_CA;
    
    
    
function reset_CA()
global values
    values.A=logical(zeros(values.sizex, values.sizey));
    values.B=values.A;
    values.C=values.A;
    values.D=values.A;
    values.T=zeros(values.sizex, values.sizey);

    

%-----------------------    
function drawfield
global t values transponders waves traces circle

    cla;
    hold on;
    
    if values.ca == 0
        if values.parab==1 & length(traces)>0
            plot(traces(:,1),traces(:,2),'r.');
        end;
    
        for i=1:length(waves)
            plot(waves{i}.center(1) + waves{i}.radius*circle(:,1), ...
                 waves{i}.center(2) + waves{i}.radius*circle(:,2), waves{i}.color);
        end;
    else
        image(256*(values.A+values.B+values.C+values.D+values.T)');
        colormap(1-gray);
        set(gca,'YDir','normal')
    end;
    
    for i=1:length(transponders)
        plot(transponders{i}.center(1),transponders{i}.center(2), [transponders{i}.color '.']);
    end;
    
    axis equal;
    text(5,5,['t=' num2str(t)]);
    axis([0 values.sizex 0 values.sizey]);
    
% -----------------------    
function update(t)
    global values transponders waves traces
       
    if values.ca>0 % must use CA waves
        values.A(2:end-1, 2:end) = values.A(1:end-2,1:end-1) | values.A(2:end-1,1:end-1) | values.A(3:end,1:end-1);
        values.B(1:end-1, 2:end-1) = values.B(2:end, 1:end-2) | values.B(2:end, 2:end-1) | values.B(2:end, 3:end);
        values.C(2:end, 2:end-1) = values.C(1:end-1, 1:end-2) | values.C(1:end-1, 2:end-1) | values.C(1:end-1,3:end);
        values.D(2:end-1, 1:end-1) = values.D(1:end-2,2:end) | values.D(2:end-1,2:end) | values.D(3:end,2:end);
        
        % Check whether any transponder fires a wave
        for i=1:length(transponders)
            if any(transponders{i}.pulse == t)
                transponders{i}.center=round(transponders{i}.center);
                values.A(transponders{i}.center(1),transponders{i}.center(2))=1;
                values.B(transponders{i}.center(1),transponders{i}.center(2))=1;
                values.C(transponders{i}.center(1),transponders{i}.center(2))=1;
                values.D(transponders{i}.center(1),transponders{i}.center(2))=1;
            end;
        end;

        for i=1:length(transponders)
            transponders{i}.center=round(transponders{i}.center);
            if sum(values.A(transponders{i}.center(1)+(-1:1),transponders{i}.center(2)))+sum(values.B(transponders{i}.center(1),transponders{i}.center(2)+(-1:1)))+sum(values.C(transponders{i}.center(1),transponders{i}.center(2)+(-1:1)))+sum(values.D(transponders{i}.center(1)+(-1:1),transponders{i}.center(2))) >= 6
                % now check that it is not in the refractory period
                if all( abs(transponders{i}.pulse - t ) > values.delay )
                    transponders{i}.pulse = [transponders{i}.pulse, t + values.delay]; % will fire below
                end;     
            end;
        end;
        
        return;
    end;
    
    
    
    % First, propagate all the waves
    for i=1:length(waves)
        waves{i}.radius = waves{i}.radius + 1; 
    end;

    % Next, find all intersections
    intersect = [];
    
    for i=1:length(waves)
        for j=i+1:length(waves)
            c = waves{i}.center-waves{j}.center;
            c = c(1) + 1i*c(2);
            r=waves{i}.radius;
            R=waves{j}.radius;
            D=(R^2-r^2-abs(c)^2)^2 - 4*r^2*abs(c)^2;
            if D<=0
                x = (R^2-r^2-abs(c)^2 + D^0.5)/(2*r*conj(c));
                p1 = waves{i}.center + r*[real(x), imag(x)];
                x = (R^2-r^2-abs(c)^2 - D^0.5)/(2*r*conj(c));
                p2 = waves{i}.center + r*[real(x), imag(x)];
                intersect = [intersect; p1; p2];
            end;  
        end;
    end;
    
    
    % Find whether a new transponder is excited, i.e., it is on an intersection
    
    if size(intersect,1)>0
        for i=1:length(transponders)
            c = transponders{i}.center;
            if any( abs( (c(1)-intersect(:,1)).^2+ (c(2)-intersect(:,2)).^2) < values.tolerance)
                % now check that it is not in the refractory period
                if all( abs(transponders{i}.pulse - t ) > values.delay )
                    transponders{i}.pulse = [transponders{i}.pulse, t + values.delay]; % will fire below
                end;
            end;
        end;
    end;
    
    % Check whether any transponder fires a wave
    for i=1:length(transponders)
        if any(transponders{i}.pulse == t)
            waves{end+1}.center=transponders{i}.center;
            waves{end}.radius=0;
            waves{end}.color = transponders{i}.color;
        end;
    end;
    
    if size(intersect,1) > 0
        ind = find( intersect(:,1)>0 & intersect(:,1)<values.sizex & intersect(:,2)>0 & intersect(:,2)<values.sizey  );
        traces = [traces;intersect(ind,:)];      
    end;

    
    % Proone waves (remove those that are outside of the visible area)   
    
    keep_waves = 1:length(waves);
    
    for i=1:length(waves)
        if waves{i}.radius > values.sizex+values.sizey
            keep_waves = keep_waves(find(keep_waves ~= i)); % exclude
        end;
    end;
    
    j=1;
    newwaves={};
    for i=keep_waves
        newwaves{j}=waves{i};
        j=j+1;
    end;
    waves=newwaves;
    
    
    
    
%-----------------------
function start
global buttons transponders
global Stop_flag values
global t

if Stop_flag == 0 % already running, needs to pause
    Stop_flag = 0.5; % pause
    values.start_pause = 'Resume';
    set(buttons.start, 'String',values.start_pause);
    return;
end;


if Stop_flag == 0.5 % resuming after a pause, i.e., starting fresh
    % keep t
else
    t=0;
    
    for i=1:length(transponders)
        transponders{i}.pulse = eval(transponders{i}.pulses);
    end;
    
end;    

values.start_pause = 'Pause';
set(buttons.start, 'String',values.start_pause);

Stop_flag = 0;

while Stop_flag == 0
    update(t);
    drawfield;
    t=t+1;
    pause(0.5*(1-values.speed));
    drawnow;
end;



    
    
function stop_clear
global t Stop_flag traces waves values buttons

traces = [];
waves = {};
Stop_flag = 1;
t = 0;
reset_CA;

values.start_pause = 'Start';


    
    
    
    
%-----------------------
function transponder_add
global transponders 

x=ginput(1);

if length(x)>0
    i=length(transponders)+1;
    transponders{i}.center = [x(1), x(2)];
    transponders{i}.pulses = '-1'; % do not fire
    transponders{i}.pulse = []; % in case it is added in the middle of simulation
    transponders{i}.color='k';
end;

set_pulses(i);


function imin=find_closest_transponder(x)
global transponders values

    dmin = values.sizex^2+values.sizey^2;
    imin=0;
    for i=1:length(transponders)
        d = sum((transponders{i}.center-x).^2);
        if d< dmin
            dmin=d;
            imin=i;
        end;
    end;


%-----------------------
function transponder_remove
global transponders values

x=ginput(1);

if length(x)>0
    
    imin = find_closest_transponder(x);
    
    j=1;
    newtransponders={};
    for i=[1:imin-1, imin+1:length(transponders)]
        newtransponders{j}=transponders{i};
        j=j+1;
    end;
    transponders = newtransponders;
end;



%-----------------------
function transponder_move
global transponders

x=ginput(1);

if length(x)>0
    i=find_closest_transponder(x);
    hold on; 
    plot(transponders{i}.center(1), transponders{i}.center(2),'ro');
    x = ginput(1);
    if length(x) > 0
        transponders{i}.center=x;
    end;
end;

%-----------------------
function set_pulses(i)
global transponders

if nargin==0 % need to find the point
    x=ginput(1);
    if length(x)>0
        i=find_closest_transponder(x);
    else
        return;
    end;
end;

    hold on; 
    plot(transponders{i}.center(1), transponders{i}.center(2),'ro');
    
    tp = inputdlg({'Provide a MATLAB expression that sets the timing of pulses of this transponder',...
                    'Provide the color of the wave (k-black, b-blue, r-red, g-green, y-yellow)'},...
                    'Times of pulses', 1, {transponders{i}.pulses, transponders{i}.color});
    if ~isempty(tp)
        transponders{i}.pulses= tp{1};
        transponders{i}.pulse = eval(tp{1});    % in case added in the middle of a simulation
        transponders{i}.color = tp{2};
    end;

    

function save_data
global t values waves transponders Stop_flag traces

fname = uiputfile('*.mat','Save Workspace As');
save(fname);


function load_data
global t values waves transponders Stop_flag traces

fname = uigetfile('*.mat','Load Workspace');;
load(fname);

if length(waves)>0  % for backward compatibility
    values.ca=0;
    disp('resetting CA');
    reset_CA;
else
    waves={};
end;

%-----------------------

function set_vals()
global buttons values;

x  = get(buttons.sizex,'string');
values.sizex = eval(x);

y  = get(buttons.sizey,'string');
values.sizey = eval(y);

d = get(buttons.delay,'string');
values.delay = eval(d);

values.speed = get(buttons.speed,'val');
values.parab = get(buttons.parab,'val');
values.ca = get(buttons.ca,'val');
reset_CA;

%-----------------------
function drawbuttons
global buttons values;

if isempty(values)
    initialize;
end;


if isempty(findobj('tag','quit')) % no figure exists 
    set(gcf, 'doublebuffer','on','Units','normalized');
    set(gcf, 'Position',[0.05 0.15 0.8 0.8]);
end;
clf
set(gca, 'Position', [0.05 0.05 0.825 0.9]);



% setup

uicontrol( ...
        'Style','frame', ...
        'Units','normalized', ...
        'Position',[0.89 0.8 0.1  0.15]);


uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','Setup',...
        'Position',[0.895 0.92 0.09  0.025]);
    
uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','Field size:',...
        'Position',[0.895 0.9 0.09  0.025]);

uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','x:',...
        'Position',[0.895 0.88 0.01  0.025]);

buttons.sizex=  uicontrol( ...
        'Style','edit', ...
        'Units','normalized', ...
        'Position',[0.905 0.88 0.03  0.025], ...
        'Interruptible','on',...
        'string',num2str(values.sizex),...
        'Value',0,...
        'tag','sizex',...
        'Callback','polychrony(''set_vals'');');

buttons.ca=uicontrol( ...
        'Style','checkbox', ...
        'Units','normalized', ...
        'Position',[0.9 0.805 0.08  0.03], ...
        'Interruptible','on',...
        'string','CA waves',...
        'Value',values.ca,...
        'tag','ca',...
        'Callback','polychrony(''set_vals'');');

    
    
uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','y:',...
        'Position',[0.94 0.88 0.01  0.025]);
buttons.sizey=  uicontrol( ...
        'Style','edit', ...
        'Units','normalized', ...
        'Position',[0.95 0.88 0.03  0.025], ...
        'Interruptible','on',...
        'string',num2str(values.sizey),...
        'Value',0,...
        'tag','sizey',...
        'Callback','polychrony(''set_vals'');');
    
    
uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','Pulse delay:',...
        'Position',[0.895 0.83 0.06  0.03]);

buttons.delay=  uicontrol( ...
        'Style','edit', ...
        'Units','normalized', ...
        'Position',[0.96 0.84 0.02  0.025], ...
        'Interruptible','on',...
        'string',num2str(values.delay),...
        'Value',0,...
        'tag','sizey',...
        'Callback','polychrony(''set_vals'');');
    
    
    
    
    

% Simulation
uicontrol( ...
        'Style','frame', ...
        'Units','normalized', ...
        'Position',[0.89 0.4 0.1  0.18]);
    
uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','Simulation speed',...
        'Position',[0.895 0.545 0.09  0.025]);

    
buttons.speed=uicontrol( ...
        'Style','slider', ...
        'Min',0,'Max',1,...
        'SliderStep',[0.01 0.1],...
        'Units','normalized', ...
        'Position',[0.895 0.52 0.09  0.02], ...
        'Value',values.speed,...
        'Interruptible','on',...
        'Callback',['polychrony(''set_vals'');']);


buttons.parab=uicontrol( ...
        'Style','checkbox', ...
        'Units','normalized', ...
        'Position',[0.9 0.485 0.08  0.03], ...
        'Interruptible','on',...
        'string','Show traces',...
        'Value',values.parab,...
        'tag','parab',...
        'Callback','polychrony(''set_vals'');');

buttons.start=uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.45 0.08  0.03], ...
        'Interruptible','on',...
        'string',values.start_pause,...
        'Value',0,...
        'tag','start',...
        'Callback','polychrony(''start'');');
    
buttons.stop=uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.41 0.08  0.03], ...
        'Interruptible','on',...
        'string','Reset',...
        'Value',0,...
        'tag','reset',...
        'Callback','polychrony(''stop_clear'');');
    
    

% transponders 
uicontrol( ...
        'Style','frame', ...
        'Units','normalized', ...
        'Position',[0.89 0.2 0.1  0.19]);

uicontrol( ...
        'Style','text', ...
        'Units','normalized', ...
        'string','transponder',...
        'Position',[0.895 0.36 0.09  0.025]);

uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.33 0.08  0.03], ...
        'Interruptible','on',...
        'string','Add',...
        'Value',0,...
        'tag','transponder_add',...
        'Callback','polychrony(''transponder_add'');');
    
uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.29 0.08  0.03], ...
        'Interruptible','on',...
        'string','Remove',...
        'Value',0,...
        'tag','transponder_remove',...
        'Callback','polychrony(''transponder_remove'');');
    
uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.25 0.08  0.03], ...
        'Interruptible','on',...
        'string','Move',...
        'Value',0,...
        'tag','transponder_move',...
        'Callback','polychrony(''transponder_move'');');

uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.21 0.08  0.03], ...
        'Interruptible','on',...
        'string','Properties',...
        'Value',0,...
        'tag','set_pulses',...
        'Callback','polychrony(''set_pulses'');');


% save, load, quit 
uicontrol( ...
        'Style','frame', ...
        'Units','normalized', ...
        'Position',[0.89 0.05 0.1  0.13]);


uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.14 0.08  0.03], ...
        'Interruptible','on',...
        'string','Save',...
        'Value',0,...
        'tag','save_data',...
        'Callback','polychrony(''save_data'');');

uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.1 0.08  0.03], ...
        'Interruptible','on',...
        'string','Load',...
        'Value',0,...
        'tag','load_data',...
        'Callback','polychrony(''load_data'');');


uicontrol( ...
        'Style','pushbutton', ...
        'Units','normalized', ...
        'Position',[0.9 0.06 0.08  0.03], ...
        'Interruptible','on',...
        'string','Quit',...
        'Value',0,...
        'tag','quit',...
        'Callback','quit;');

   