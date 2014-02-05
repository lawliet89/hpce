function [] = animate_julia_v1( w, h, maxiter )

if nargin<1
   w=128; 
end
if nargin<2
    h=w;
end
if nargin < 3
    maxiter=16;
end

% Set up the color mapping to highlight changes
m=jet(32);
m=repmat(m,ceil(maxiter/32),1);
m=m(1:maxiter,:);
m(end,:)=[0 0 0];
colormap(m);

% Needed when controlling with mouse movement
set (gcf, 'WindowButtonMotionFcn', @(a,b)(0));

% Sets up default animation
cScale=0.9;
cSpeed=0.05;

axis equal;

id=tic();
t=0;
while 1
    % Rotate c around a circle based on elapsed time
    c = sin(t*cSpeed*2*pi) + cos(t*cSpeed*2*pi)*i;
    c = c*cScale;
    
    % Or take c from current mouse location 
    CP = get (gca, 'CurrentPoint');
    mx=(CP(1,1)/w)*2-1;
    my=(CP(1,2)/h)*2-1;
    m=mx+my*1i;
    if abs(m)<=1
        c=m;
    end
    
    % Render the actual pixels
    pixels=render_julia_v1(w, h, c, maxiter);
    t = toc(id);
    
    % And show them.
    image(pixels);
    drawnow;    
end

end

