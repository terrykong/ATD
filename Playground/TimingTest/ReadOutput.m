nCellNum = 7;
nDataNum = 8;

timefac = 6;

filename = 'output.txt';
delimiter = ' ';
A = importdata(filename,delimiter);
[r,c] = size(A);
B = zeros(r,2);
B(:,1) = A(:,1);
for i = 1:r
    B(i,2) = mean(A(i,2:end))/B(i,1)*timefac;
end

% Plot
colors = distinguishable_colors(nCellNum);
markers = 'p+o*xsdvh';
clf;
for i = 1:nCellNum
    h = loglog(B( (1+nDataNum*(i-1)):(nDataNum*i) , 1) , ...
             B( (1+nDataNum*(i-1)):(nDataNum*i) , 2) , ...
             'color', colors(i,:), ...
             'marker',markers(i));
         hold on;
end

% plot 10^-6 line (microsecond)
where = 1e-6;
x = linspace(1,100000,100);
y = where*ones(1,length(x));
loglog(x,y,'--');
text(10000,where,'1\mus','fontsize',18,'verticalalignment','bottom')

where = 2e-7;
x = linspace(1,100000,100);
y = where*ones(1,length(x));
loglog(x,y,'k-.');
text(10000,where,'0.2\mus','fontsize',18,'verticalalignment','top')


legend('5 cells','10 cells','20 cells','50 cells','100 cells','200 cells','500 cells')
xlabel('Number of Datum')
ylabel('Time (s)')
