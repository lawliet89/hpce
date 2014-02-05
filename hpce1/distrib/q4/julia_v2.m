function [viter,vz] = julia_v2(vz, c, maxiter)

    iter = 1;
    viter = ones(1, size(vz, 2));
    while iter < maxiter
        mask = abs(vz) <= 1;
        if ~any(mask)
            break;
        end
        iter = iter + 1;
        vz(mask) = vz(mask).^2 + c;
        viter(mask) = iter;
    end

end
