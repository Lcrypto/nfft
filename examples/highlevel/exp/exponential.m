k=1.712312;

system(['./exp  '  num2str(k) '  > out.dat']);

load out.dat

figure(1)
hold on;
plot(out(:,1),out(:,2),'r-');
plot(out(:,1),out(:,3),'b-');
%plot(out(:,1),out(:,6),'g-');
hold off;

figure(2)
hold on;
plot(out(:,1),out(:,4),'r-');
plot(out(:,1),out(:,5),'b-');
%plot(out(:,1),out(:,6),'g-');
hold off;
