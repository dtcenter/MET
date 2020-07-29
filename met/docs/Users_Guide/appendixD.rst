.. _appendixD:

.. _App_D-Confidence-Intervals:

Appendix D Confidence Intervals
===============================

A single verification statistic is statistically meaningless without associated uncertainty information in accompaniment. There can be numerous sources of uncertainty associated with such a statistic including observational, physical uncertainties about the underlying processes governing the equation, sample uncertainty, etc. Although all of the sources of uncertainty can be important, the most heavily researched, and easiest to calculate, is that of sampling uncertainty. It is this source of uncertainty that can presently be obtained with MET, and the techniques for deriving these estimates are described here. Sampling uncertainty through MET is gleaned by way of confidence intervals (CIs) as these are generally most informative. A :math:`(1 - \alpha) \cdot 100\%` confidence interval is interpreted, somewhat awkwardly, in the following way. If the test were repeated 100 times (so that we have 100 such intervals), then we expect the true value of the statistics to fall inside :math:`(1-\alpha)\cdot 100` of these intervals. For example, if :math:`\alpha=0.05` then we expect the true value to fall within 95 of the intervals.

There are two main types of CIs available with MET: parametric and non-parametric. All of the parametric intervals used with MET rely on the underlying sample (or the errors, :math:`F - O`) to be at least approximately independent and normally distributed. Future releases will allow for some types of dependency in the sample. The non-parametric techniques utilize what is known in the statistical literature as bootstrap resampling, which does not rely on any distributional assumptions for the sample; the assumption is that the sample is representative of the population. Bootstrap CIs can be inaccurate if the sample is not independent, but there are ways of accounting for dependence with the bootstrap, some of which will be added to MET in future releases. Details about which verification statistics have parametric CIs in MET are described next, and it should be noted that the bootstrap can be used for any statistic, though care should be taken in how it is carried out, and this is described subsequently.

The most commonly used confidence interval about an estimate for a statistic (or parameter), :math:`\theta`, is given by the normal approximation

.. math:: \theta \pm z_{\alpha / 2} \cdot V(\theta )

where :math:`z_{\alpha / 2}` is the :math:`\alpha - \text{th}` quantile of the standard normal distribution, and :math:`V(\theta )` is the standard error of the statistic (or parameter), :math:`\theta`. For example, the most common example is for the mean of a sample, :math:`X_1,\cdots,X_n`, of independent and identically distributed (iid) normal random variables with mean :math:`\mu` and variance :math:`\sigma`. Here, the mean is estimated by :math:`\frac{1}{n} \sum_{i=1}^n X_i = \bar{X}`, and the standard error is just the standard deviation of the random variables divided by the square root of the sample size. That is, :math:`V( \theta ) = V ( \bar{X} ) = \frac{\sigma}{\sqrt{n}}`, and this must be estimated by :math:`\hat{V} (\bar{X} )`, which is obtained here by replacing :math:`\sigma` by its estimate, :math:`\hat{\sigma}`, where :math:`\hat{\sigma} = \frac{1}{n - 1} \sum_{i=1}^n (X_i - \bar{X})^2`.

Mostly, the normal approximation is used as an asymptotic approximation. That is, the interval for :math:`\theta` may only be appropriate for large **n**. For small **n**, the mean has an interval based on the Student’s **t** distribution with **n-1** degrees of freedom. Essentially, :math:`z_{\alpha / 2}` of the question is replaced with the quantile of this **t** distribution. That is, the interval is given by 

.. math:: \mu \pm t_{\alpha / 2,\nu - 1} \cdot \frac{\sigma}{\sqrt{n}}

where again, :math:`\sigma` is replaced by its estimate, :math:`\hat{\sigma}`, as described above.

:numref:`verif_stat_approx_CI` summarizes the verification statistics in MET that have normal approximation CIs given by :math:`\theta` along with their corresponding standard error estimates, . It should be noted that for the first two rows of this table (i.e., Forecast/Observation Mean and Mean error) MET also calculates the interval around :math:`\mu` for small sample sizes.

.. _verif_stat_approx_CI:

.. list-table:: Verification statistics with normal approximation CIs given by the equation for :math:`\theta` provided in MET along with their associated standard error estimate.
  :widths: auto
  :header-rows: 1

  * - :math:`\hat{\theta}`
    - :math:`V(\theta)`
  * - Forecast / Observation Mean
    - :math:`V(\bar{X}) = \frac{\sigma_x}{\sqrt{n}}` where :math:`\sigma_x` emphasizes that this is the estimated standard deviation of the underlying sample.
  * - Mean error
    - :math:`V(\bar{F} - \bar{O}) = \frac{\sigma_{F - O}}{\sqrt{n}}`, where :math:`\sigma_{F - O}` emphasizes that this is the estimated standard deviation of the errors, :math:`F - O`.
  * - Brier Score (BS)
    - :math:`V(\text{BS}) = \frac{1}{T} [\sum F^4 + \bar{O} (1 - 4 \sum F_{F | O=1}^3 + 6 \sum F_{F | O=1}^2 - 4 \sum F_{F | O=1}) - \text{BS}^2]` where **F** is the **probability** forecast and **O** is the observation. See :ref:`Bradley et al, 2008 <Bradley-2008>` for derivation and details.
  * - Peirce Skill Score (PSS)
    - :math:`V(\text{PSS}) = \sqrt{\frac{H(1 - H)}{n_H} + \frac{F(1 - F)}{n_F}}`, where **H** is the hit rate, **F** the false alarm rate, :math:`n_h` the number of hits and misses, and :math:`n_F` the number of false alarms and correct negatives.
  * - Logarithm of the odds ratio (OR)
    - :math:`V(\ln(\text{OR})) = \sqrt{\frac{1}{a} + \frac{1}{b} + \frac{1}{c} + \frac{1}{d}}`, where the values in the denominators are the usual contingency table counts.


Other statistics in MET having parametric CIs that rely on the underlying sample to be at least approximately iid normal, but have a different form derived from the normality assumption on the sample include the variance, standard deviation, and the linear correlation coefficient. These are addressed subsequently.

Generally, the normal interval around :math:`\theta` is appropriate for statistics of continuous variables, but a limit law for the binomial distribution allows for use of this interval with proportions. The most intuitive estimate for :math:`V(\theta )` in this case is given by :math:`V(p) = \sqrt{\hat{p} (1 - \hat{p}) / n}`. However, this only applies when the sample size is large. A better approximation to the CI for proportions is given by Wilson’s interval, which is 

.. math:: \frac{\hat{p} + z_{\alpha / 2}^2 + z_{\alpha / 2} \sqrt{\hat{p} (1 - \hat{p}) / 4n}}{1 + z_{\alpha / 2}^2 / n}

where :math:`\hat{p}` is the estimated proportion (e.g., hit rate, false alarm rate, PODy, PODn, etc.). Because this interval around :math:`\hat{p}` generally works better than the more intuitive normal approximation interval for both large and small sample sizes, this is the interval employed by MET.

The forecast/observation variance has CIs derived from the underlying sample being approximately iid normal with mean :math:`\mu` and variance :math:`\sigma`. The lower and upper limits for the interval are given by

.. math:: l(\sigma^2) = \frac{(n - 1)s^2}{\chi_{\alpha / 2,n - 1}^2} \text{ and } u(\sigma^2) = \frac{(n - 1)s^2}{\chi_{1 - \alpha / 2, n - 1}^2}


respectively, where :math:`\chi_{\alpha , \nu}^2` is the :math:`\alpha - \text{th}` quantile of the chi-square distribution with **n-1** degrees of freedom. Taking the square roots of the limits of :math:`l` yields the CI for the forecast/observation standard deviation.

Finally, the linear correlation coefficient has limits given by 

.. math:: (\frac{e^{2c_l} - 1}{e^{2c_l} + 1}, \frac{e^{2c_u} - 1}{e^{2c_u} + 1})


where :math:`c_l = v - \frac{z_{\alpha / 2}}{\sqrt{n - 3}}` and :math:`c_u = v + \frac{z_{\alpha / 2}}{\sqrt{n - 3}}`.

All other verification scores with CIs in MET must be obtained through bootstrap resampling. However, it is also possible to obtain bootstrap CIs for any of the statistics given above, and indeed it has been proven that the bootstrap intervals have better accuracy for the mean than the normal approximation. The bootstrap algorithm is described below.

1. Assume the sample is representative of the population. 

2. Resample with replacement from the sample (see text below). 

3. Estimate the parameter(s) of interest for the current replicated sample. 

4. Repeat steps 2 and 3 numerous times, say B times, so that you now have a sample of size B of the parameter(s). 

5. Calculate CIs for the parameters directly from the sample (see text below for more details)

Typically, a simple random sample is taken for step 2, and that is how it is done in MET. As an example of what happens in this step, suppose our sample is :math:`X_1,X_2,X_3,X_4`. Then, one possible replicate might be :math:`X_2,X_2,X_2,X_4`. Usually one samples :math:`m = n` points in this step, but there are cases where one should use :math:`m < n`. For example, when the underlying distribution is heavy-tailed, one should use a smaller size m than n (e.g., the closest integer value to the square root of the original sample size).

There are numerous ways to construct CIs from the sample obtained in step 4. MET allows for two of these procedures: the percentile and the BCa. The percentile is the most commonly known method, and the simplest to understand. It is merely the :math:`\alpha / 2` and :math:`1 - \alpha / 2` percentiles from the sample of statistics. Unfortunately, however, it has been shown that this interval is too optimistic in practice (i.e., it doesn’t have accurate coverage). One solution is to use the BCa method, which is very accurate, but it is also computationally intensive. This method adjusts for bias and non-constant variance, and yields the percentile interval in the event that the sample is unbiased with constant variance.

If there is dependency in the sample, then it is prudent to account for this dependency in some way. One method that does not make a lot of assumptions is circular block bootstrapping. This is not currently implemented in MET, but will be available in a future release. At that time, the method will be explained more fully here, but until then consult :ref:`Gilleland (2010) <Gilleland-2010>` for more details.
