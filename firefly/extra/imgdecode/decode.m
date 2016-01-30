im = imread('AAF00F55-1.jpg');

                                # seperate the colors out
imr = im(:,:,1);
img = im(:,:,2);
imb = im(:,:,3);

                                # Expect the colors to be horizontal bands, sum along these
imr = sum(imr,2).^2;
img = sum(img,2).^2;
imb = sum(imb,2).^2;

                                # Get rid of offset via averaging
avglen = length(imr);
imr = imr - filter(ones(avglen,1)/avglen, 1, imr);
img = img - filter(ones(avglen,1)/avglen, 1, img);
imb = imb - filter(ones(avglen,1)/avglen, 1, imb);

#plot(imr,'r')
#figure
                                #plot(img,'g')
                                #figure
                                #plot(imb,'b')
                                # apply thresholding to data in order to get binary out.
imr = imr > 0;
img = img > 0;
imb = imb > 0;

#plot(imr,'r*')
#figure
                                #hold on
                                #plot(img,'g')
                                #plot(imb,'b')
                                #hold off

sync = ~(imr | img | imb);
#plot(sync,'k.')

                                # scan through sync data, remove glitches
length_threshold = length(sync)/150
startidx = [1];
endidx = [];
curval = sync(1);
for i = 1:length(sync)
  if sync(i) == curval
    continue
  end
                                # value changed!
  endidx = [endidx i-1];
  if endidx(end)-startidx(end) < length_threshold
    sync(startidx(end):endidx(end)) = sync(i);
    if length(startidx) ~= 1
      startidx(end) = [];
    end

    endidx(end) = [];
    if length(endidx) ~= 0
      endidx(end) = [];
    end
  else
    startidx = [startidx i];
  end
  curval = sync(i);
end
                                # get the lengths of contiguous bit segments
combined = [startidx(1:end-1); endidx; endidx-startidx(1:end-1)]
                                # assume more sync bits than data sections, take median, use this as bit length
                                #combined = sortrows(combined',3)';
median(combined(3,:))
bitlen = median(combined(3,:));

packets = combined(:,combined(3,:) > 9*bitlen*0.8 & combined(3,:) < 9*bitlen*1.2)
if columns(packets) == 0
  error('No packets found!')
end

# add all the packets up to get a better snr (alternative, just choose longest one, its probably just as good.)
fr = imr(packets(1,1):packets(2,1));
fg = img(packets(1,1):packets(2,1));
fb = imb(packets(1,1):packets(2,1));
for i = 2:columns(packets)
  fr = fr + [imr(packets(1,1):packets(2,1))];
  fg = fg + [img(packets(1,1):packets(2,1))];
  fb = fb + [imb(packets(1,1):packets(2,1))];
end

fr = fr > columns(packets)/2;
fg = fg > columns(packets)/2;
fb = fb > columns(packets)/2;
#plot(fr,'r.')
#figure
#plot(fg,'g.')
#figure
#plot(fb,'b.')

byteR = 0
byteG = 0
byteB = 0

bitlen = floor((length(fr))/9) # 8 bits plus one bit from preamble
for i = 1:bitlen:bitlen*8
  byteR = byteR*2 + (mean(fr(i:i+bitlen)) > 0.5);
  byteG = byteG*2 + (mean(fg(i:i+bitlen)) > 0.5);
  byteB = byteB*2 + (mean(fb(i:i+bitlen)) > 0.5);
end
byteR
dec2hex(byteR)
byteG
dec2hex(byteG)
byteB
dec2hex(byteB)
