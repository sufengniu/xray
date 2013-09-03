% pseudo-code to calculate mean and standard deviation of a series of dark images.
% The function getImg() is not included (or written!).  getImg(filename, index) returns image
% number index from file filename when supplied with a filename and index

clear
clc
% common setup for all 3 methods
filename = '55Fe_run5_dark.tif';
firstDark = 1;
lastDark = 44;
numDark = lastDark - firstDark + 1;
% end of setup

info = imfinfo(filename);
num_images = numel(info);

for k = 1:num_images
    A(:,:,k)=imread(filename, k, 'Info', info);
end

% method 1 - demonstrates calculation explicitly but requires 2 loops
dk = A(:,:,firstDark);
DKave1=zeros(size(dk));
for ii = firstDark:lastDark
    DKave1 = double(DKave1) + double(A(:,:,ii));
end
DKave1 = DKave1/numDark;

DKstd1=zeros(size(dk));
for ii = firstDark:lastDark
    DKstd1 = double(DKstd1) + (double(A(:,:,ii))-double(DKave1)).^2;
end
DKstd1 = DKstd1/(numDark - 1);
DKstd1 = sqrt(DKstd1);
% end method 1

% method 2 - use Matlab built-in functions and a 3-D stack of images -
% probably does not scale well with large stacks
dk = A(:,:,firstDark);
dkImgs = zeros(size(dk,1), size(dk, 2), numDark);
for ii = firstDark:lastDark
    dkImgs(:,:,ii) = A(:,:,ii);
end
DKave2 = mean(dkImgs,3);
%DKstd2 = std(dkImgs,3);
% end method 2

% method 3 - calculate two quantities in a single loop that can combine to determine mean and standard deviation 
dk0 = A(:,:,firstDark); % save for later
A1 = zeros(size(dk0));
A2 = zeros(size(dk0));
for ii = firstDark+1:lastDark
    dk = A(:,:,ii);
    A1 = double(A1) + (double(dk) - double(dk0));
    A2 = double(A2) + (double(dk) - double(dk0)).^2;
end
DKave3=(A1/double(numDark))+double(dk0);
DKstd3=sqrt((double(A2)-(A1.^2/double(numDark)))/double(numDark-1));
% end method 3

%-end of calculations - rest is just plotting results for comparison

% plot and compare results - could also do subtractions and see what is
% left, if anything
minDKave = min([min(DKave1(:)) min(DKave2(:)) min(DKave3(:))]);
maxDKave = max([max(DKave1(:)) max(DKave2(:)) max(DKave3(:))]);
minDKstd = min([min(DKstd1(:))  min(DKstd3(:))]);
maxDKstd = max([max(DKstd1(:))]);

figure(1);
subplot(2,2,1)
imagesc(DKave1, [minDKave maxDKave]);
axis image;
title('BRUTE FORCE AVERAGE');

subplot(2,2,2)
imagesc(DKave2, [minDKave maxDKave]);
axis image;
title('MATLAB AVERAGE');

subplot(2,2,3)
imagesc(DKave3, [minDKave maxDKave]);
axis image;
title('SINGLE-LOOP AVERAGE');

figure(2);
subplot(2,2,1)
imagesc(DKstd1, [minDKstd maxDKstd]);
axis image;
title('BRUTE FORCE RMS');

% subplot(2,2,2)
% imagesc(DKstd2, [minDKstd maxDKstd]);
% axis image;
% title('MATLAB RMS');

subplot(2,2,3)
imagesc(DKstd3, [minDKstd maxDKstd]);
axis image;
title('SINGLE-LOOP AVERAGE');