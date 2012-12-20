% create a sphere with a radius of 5um and a refractive index of 1.5

% Start with
% DDA/DDA --geometry load:file=Matlab/Geometry.hdf5 --efield

% Equivalent to:
% DDA/DDA --geometry sphere:5um,1.5 --grid-unit 0.5um --efield


gridSpacing = 0.5e-6; % 0.5um
r = 5e-6;
m = 1.5;

s = struct;
s.Type = 'Geometry';
s.Geometry.Type = 'DDADipoleList';
s.Geometry.GridSpacing = [gridSpacing; gridSpacing; gridSpacing];
s.Geometry.RefractiveIndices = [m; m; m];

maxLen = (ceil(r/gridSpacing) - floor(-r/gridSpacing) + 1) ^ 3;
len = 0;
s.Geometry.DipolePositions = zeros (3, maxLen);
s.Geometry.DipoleMaterialIndices = zeros (1, maxLen);
for z = floor(-r/gridSpacing):ceil(r/gridSpacing)
  for y = floor(-r/gridSpacing):ceil(r/gridSpacing)
    for x = floor(-r/gridSpacing):ceil(r/gridSpacing)
      v = [ x y z ];
      v2 = v * gridSpacing;
      v = v - [ floor(-r/gridSpacing) floor(-r/gridSpacing) floor(-r/gridSpacing) ];
      if (v2 * v2' <= r * r)
        len = len + 1;
        s.Geometry.DipolePositions (:, len) = v';
        s.Geometry.DipoleMaterialIndices (len) = 0;
      end
    end
  end
end
s.Geometry.DipolePositions = s.Geometry.DipolePositions (:, 1:len);
s.Geometry.DipoleMaterialIndices = s.Geometry.DipoleMaterialIndices (1:len);


if exist('octave_config_info') 
  save -hdf5 'Geometry.hdf5' -struct s
else
  save -v7.3 'Geometry.hdf5' -struct s
end

% Local Variables: 
% mode: octave
% End: 
