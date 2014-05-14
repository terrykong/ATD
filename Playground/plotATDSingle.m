function plotATDSingle(nPoints,PW,data,isFound,threshold,cutCellIndex,suppressPlot,constant)
% This is a dynamic plot box that plots nPoints at a time
%
% Input:
%   - nPoints: number of points to plot at a time
%   - PW: pulse width
%   - data: data to plot
%   - isFound: 0 = not found, 1 = found
%   - threshold: threshold from ATD

persistent figureHandle dataHandle thresholdHandle legendHandle;
persistent ATD_x ATD_y ATD_threshold foundTable foundHandle;
persistent delayWindow delay;
persistent constantThreshHandle;

mylinewidth = 1.5;

if nargin < 5
    threshold = [];
end

if nargin < 7
    suppressPlot = false;
end

if nargin < 8
    constant = [];
end

if isempty(figureHandle)
    figureHandle = figure;
else
    figure(figureHandle);
end

if isempty(ATD_x)
    ATD_x = PW*(-nPoints:-1);
end

if isempty(ATD_y)
    ATD_y = zeros(1,nPoints);
end

if isempty(ATD_threshold)
    ATD_threshold = zeros(1,nPoints);
end

if ~iscell(foundTable)
    foundTable = {};
end

if isempty(delay)
    delay = cutCellIndex + 1;
end

if isempty(delayWindow)
    delayWindow = zeros(1,cutCellIndex);
end

if delay > 0
    delay = delay - 1;
    delayWindow = [data delayWindow(1:end-1)];
%     sig = 0;
%     thr = 0;
    return;
else
    temp = delayWindow(end);
    delayWindow = [data delayWindow(1:end-1)];
    data = temp;
end

%% Actual Plot
ATD_x = [ATD_x(2:end) ATD_x(end)+PW];
ATD_y = [ATD_y(2:end) data];
ATD_threshold = [ATD_threshold(2:end) threshold];
% sig = ATD_y(1);
% thr = ATD_threshold(1);
if suppressPlot
    if isFound
        foundTable{end+1} = [ATD_x(end),double(data)];
        foundHandle = [foundHandle plot(ATD_x(end),data,'go','markerfacecolor','g','markersize',15)];
    end
    return;
end

if isempty(get(figure(figureHandle),'children'))
    hold on;
    dataHandle = plot(ATD_x,ATD_y,'b','linewidth',mylinewidth);
    thresholdHandle = plot(ATD_x,ATD_threshold,'r','linewidth',mylinewidth);
    constantThreshHandle = plot(ATD_x,constant*ones(1,length(ATD_x)),'m--','linewidth',mylinewidth);
    if isempty(constant)
        legendHandle = legend('raw data','adaptive threshold  ','constant threshold  ','Location','NorthEast');
        set(legendHandle,'FontSize',25);
    else
        legendHandle = legend('raw data','adaptive threshold  ','constant threshold  ','Location','NorthEast');
        set(legendHandle,'FontSize',25);
    end
    for i = 1:length(foundTable)
        plot(foundTable{i}(1),foundTable{i}(2),'go','markerfacecolor','g','markersize',15)
    end
else
    hold on;
    if isempty(dataHandle)
        dataHandle = plot(ATD_x,ATD_y,'b','linewidth',mylinewidth);
    else
        delete(dataHandle);
        dataHandle = plot(ATD_x,ATD_y,'b','linewidth',mylinewidth);
    end

    if isempty(thresholdHandle)
        thresholdHandle = plot(ATD_x,ATD_threshold,'r','linewidth',mylinewidth);
    else
        delete(thresholdHandle);
        thresholdHandle = plot(ATD_x,ATD_threshold,'r','linewidth',mylinewidth);
    end

    if isempty(constantThreshHandle)
        constantThreshHandle = plot(ATD_x,constant*ones(1,length(ATD_x)),'m--','linewidth',mylinewidth);
    else
        delete(constantThreshHandle);
        constantThreshHandle = plot(ATD_x,constant*ones(1,length(ATD_x)),'m--','linewidth',mylinewidth);
    end
    
    if isempty(legendHandle)
        if isempty(constant)
            legendHandle = legend('raw data','adaptive threshold  ','constant threshold  ','Location','NorthEast');
            set(legendHandle,'FontSize',25);
        else
            legendHandle = legend('raw data','adaptive threshold  ','constant threshold  ','Location','NorthEast');
            set(legendHandle,'FontSize',25);
        end
    end
    
    if isFound
        foundTable{end+1} = [ATD_x(end),double(data)];
        foundHandle = [foundHandle plot(ATD_x(end),data,'go','markerfacecolor','g','markersize',15)];
    end
end

deleteThese = logical(zeros(1,length(foundTable)));
for i = 1:length(foundTable)
    if foundTable{i}(1) < ATD_x(1)
        deleteThese(i) = true;
    end
end
foundTable(deleteThese) = [];
delete(foundHandle(deleteThese));
foundHandle(deleteThese) = [];
