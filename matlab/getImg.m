function [ image_array ] = getImg( filename, index )
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here
info = imfinfo(filename);
num_images = numel(info);

for k = 1:num_images
    A(:,:,k)=imread(filename, k, 'Info', info);
end

image_array=A(:,:,index);
end

