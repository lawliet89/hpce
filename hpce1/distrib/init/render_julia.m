function [pixels]=render_julia(w,h,c, maxiter)

if nargin < 3
    c = 0.5+0.5i;
end
if nargin < 4
    maxiter=16;
end

pixels=zeros(h,w);

i=sqrt(-1); % Not really necessary, defined by default

ox=-1;
dx=2/w;
oy=-1;
dy=2/h;

for x=1:w
    for y=1:h
        z = (ox+x*dx) + (oy+y*dy)*i;
        pixels(y,x) = julia(z, c, maxiter);
    end
end
