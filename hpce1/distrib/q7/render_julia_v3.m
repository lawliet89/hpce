function [pixels]=render_julia_v3(juliaproc, w,h,c, maxiter)
% render_julia_v2 : Renders julia set using a specified iteration ...
% procedure
% juliaproc : A function of the form [viter,vz] = juliaproc(vz, ...
% c, maxiter)
% where vz can be a vector.
% w,h,c,maxiter : Standard rendering controls
if nargin < 4
    c = 0.5+0.5i;
end
if nargin < 5
    maxiter=16;
end

pixels=zeros(h,w);

i=sqrt(-1); % Not really necessary, defined by default

ox=-1;
dx=2/w;
oy=-1;
dy=2/h;

y=1:h;
parfor x=1:w
    z = (ox+x*dx) + (oy+y.*dy)*i;
    pixels(y,x) = juliaproc(z, c, int32(maxiter));
end
