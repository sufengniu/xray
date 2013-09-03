clear 
clc
fname = '55Fe_run6_dark.tif';
info = imfinfo(fname);
num_images = numel(info);

for i=1:num_images
    A(:,:,i)=imread(fname, i, 'Info', info);
    
end