function [ f ] = rjv2( jp, rp )
    if nargin < 2
        rp = @render_julia_v2;
    end
    f=@(w,h,c,maxiter)( rp(jp, w,h,c,maxiter) );
end