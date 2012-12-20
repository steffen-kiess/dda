% Show the latest efield output

filename = '../DDA/output/latest/FarPlane.hdf5';

if exist('octave_config_info') 
  data = load (filename);
else
  data = load ('-mat', filename);
end

% show element S1 of the jones matrix
plot (data.JonesFarField.Theta / pi * 180, reshape (abs (data.JonesFarField.Data (2, 2, :)), [], 1))

% Local Variables: 
% mode: octave
% End: 
