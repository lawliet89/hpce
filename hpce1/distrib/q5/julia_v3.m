function [viter,vz] = julia_v3(vz, c, maxiter)

    iter = 1;
    viter = ones(1, size(vz, 2));
    while iter < maxiter
        indices = find(abs(vz) <= 1);
        if ~any(indices)
            break;
        end
        iter = iter + 1;
        vz(indices) = vz(indices).^2 + c;
        viter(indices) = iter;
    end

end
