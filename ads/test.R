
require(forecast)

data = read.csv('http://ucanalytics.com/blogs/wp-content/uploads/2015/06/Tractor-Sales.csv')

data = ts(data[,2],start = c(2003,1),frequency = 12)

plot(data, xlab='Years', ylab = 'Tractor Sales')
plot(diff(log10(data)),ylab='Differenced Tractor Sales')

par(mfrow = c(1,2))
acf(ts(diff(log10(data))),main='ACF Tractor Sales')
pacf(ts(diff(log10(data))),main='PACF Tractor Sales')

ARIMAfit = auto.arima(log10(data), approximation=FALSE,trace=FALSE)
summary(ARIMAfit)

pred = predict(ARIMAfit, n.ahead = 36)
par(mfrow = c(1,1))
plot(data,type='l',xlim=c(2004,2018),ylim=c(1,1600),xlab = 'Year',ylab = 'Tractor Sales')
lines(10^(pred$pred),col='blue')
lines(10^(pred$pred+2*pred$se),col='orange')
lines(10^(pred$pred-2*pred$se),col='orange')