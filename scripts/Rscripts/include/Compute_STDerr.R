## Function to compute the Standard Error of an uncorrelated time series.
##
## Remove the correlated portion of a time series, using a first order auto-correlation coefficient
## to help compute an inflated variance factor for the uncorrelated portion of the time series.
##
## method = "ML" or "CSS".
##
## Originator:  Eric Gilleland and Andrew Loughe, 08 JUL 2008
##

Compute_STDerr_from_median <- function ( data, method )
{

   if ( IQR(data) > 0.0 & length(data) > 2 ) {

      ## Compute the Standard Error.
      STDerr_data <- ( IQR(data) * sqrt(pi/2.) ) / ( 1.349 * sqrt( length(data) ) )

   } else {

      STDerr_data = 0.0;

   }

   return ( c(STDerr_data, 0, 0, length(data)) );

}

Compute_STDerr_from_mean <- function ( data, method )
{

   RATIO_flag = 0;     ## Problem computing the vif ratio?

   if ( var(data) > 0.0 & length(data) > 2 ) {

      ## Compute the first order auto-correlation coefficient.
      data.arima <- try(arima( data, order=c(1,0,0), method=method ))

      ## Catch errors from arima
      if(class(data.arima) == "try-error") {
         print(paste("WARNING: arima failed for dataset of length", length(data)));
         return(c(NA, NA, NA, length(data)));
      }

      ## If the AR1 coefficient is out-of-bounds, try a different ARIMA method.
      ## What about higher-order coefficients?  Should we fit a higher-order function
      ## if those coefficients are large?
      if ( coef(data.arima)[1] < 0.3 | coef(data.arima)[1] >= 0.99 ) { 
         if ( method == 'ML' ) { method = "CSS"; } else { method = "ML"; }
         data.arima <- try(arima( data, order=c(1,0,0), method=method ))
         
         ## Catch errors from arima
         if(class(data.arima) == "try-error") {
            print(paste("WARNING: different arima method failed for dataset of length", length(data)));
            return(c(NA, NA, NA, length(data)));
         }
      }

      ## Compute a variance inflation factor (having removed that portion of the time series that was correlated).
      RATIO = ( 1 + coef(data.arima)[1] ) / ( 1 - coef(data.arima)[1] )

      ## Check for a zero RATIO, that will then be operated on by SQRT.
      ## If necessary, try a different arima method, or just set RATIO to one.
      if ( RATIO < 0.0 ) {
         if ( method == 'ML' ) { method = "CSS"; } else { method = "ML"; }
         data.arima <- arima( data, order=c(1,0,0), method=method )
         RATIO = ( 1 + coef(data.arima)[1] ) / ( 1 - coef(data.arima)[1] )
      }
      if ( RATIO < 0.0 ) { RATIO = 1.0; RATIO_flag = 1; }

      variance_inflation_factor = sqrt( RATIO );

      ## If the AR1 coefficient is less than 0.3, then don't even use a vif!  Set vif = 1.0
      if ( coef(data.arima)[1] < 0.3 | coef(data.arima)[1] >= 0.99  ) { variance_inflation_factor = 1.0; }

      ## Compute the Standard Error using the variance inflation factor.
      STDerr_data <- variance_inflation_factor * sqrt( var(data) ) / sqrt( length(data) );

      AR1 = coef(data.arima)[1];

   } else {

      STDerr_data = 0.0;
      AR1 = 0.0;
   }

   return ( c(STDerr_data, RATIO_flag, AR1, length(data)) );

}
