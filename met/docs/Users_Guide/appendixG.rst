.. _appendixG:

Appendix G Vectors and Vector Statistics
========================================

In this appendix, we discuss some basic properties of vectors, concentrating on the two-dimensional case. To keep the discussion simple, we will assume we are using a Cartesian coordinate system. \def\vec#1{\hbox{\bf #1}}

Traditionally, vectors have been defined as quantities having both magnitude and direction, exemplified by a directed line segment. The magnitude of the vector is shown by the length of the segment, and the direction of the vector is usually shown by drawing an arrowhead on one end of the segment. Computers (and, in the author's experience, people) are usually more comfortable working with numbers, and so instead of the usual graphical definition of a vector, we will take the definition used in analytic geometry: A (two-dimensional) vector is an ordered pair of numbers. We will use boldface type to denote vectors, and so we can write \vec{v}=(a,b)

to show that the vector \vec{v} consists of the ordered pair of numbers $a$ and $b$. The number $a$ is called the first (or $x$) component of \vec{v}, and $b$ is called the second (or $y$) component. Vector addition is performed component-wise: $(a, b) + (c, d) = (a + c, b + d)$, and similarly for subtraction. If $\alpha$ is a scalar, then we define multiplication by the scalar $\alpha$ as $\alpha (a, b) = (\alpha a, \alpha b)$, and similarly for division by a (nonzero!) scalar.

The {\it norm} (or length, or magnitude) of a vector $\vec{v} = (a, b)$, is\left|\vec{v}\right|=\sqrt{a^{2}+b^{2}}

Note that $\left|\vec{v}\right| = 0$ if and only if $a = b = 0$, in which case we say that \vec{v} is the {\it zero vector}. If $\alpha$ is a scalar, then \left|\alpha\vec{v}\right|=\left|\alpha\right|\left|\vec{v}\right|

The most important relation between vectors and their norms is given by the {\it triangle inequality}:\left|\vec{v}+\vec{w}\right|\leq\left|\vec{v}\right|+\left|\vec{w}\right|

In some cases, only the direction of a vector is of interest, and in such cases we can replace a nonzero vector \vec{v} by the unique vector $N(\vec{v})$ that has the same direction as \vec{v}, but has norm 1:N(\vec{v})=\frac{\vec{v}}{\left|\vec{v}\right|}

The vector $N(\vec{v})$ will be called the {\it unit vector} corresponding to \vec{v}, or more simply the {\it direction} of \vec{v}. Note that the zero vector has no direction.

Since vectors are characterized by magnitude (norm) and direction, this gives two ways to compare vectors: we can compare either their magnitudes or their directions. If \vec{v} and \vec{w} are vectors, then we can compare their norms by either taking the norm of the difference $\left|\vec{v} - \vec{w}\right|$ or the difference of the norms $\left|\vec{v}\right| - \left|\vec{w}\right|$. It's not always made clear in verification studies which of these is meant, and in general these two quantities will be different. However, by making use of the triangle inequality it can be shown that there is a relation between them. To derive this, let $ \vec{z} = \vec{v} - \vec{w} $, from which we get $ \vec{v} = \vec{w} + \vec{z} $. Now taking norms and using the triangle inequality, \left|\vec{v}\right|=\left|\vec{w}+\vec{z}\right|\leq\left|\vec{w}\right|+\left|\vec{z}\right|=\left|\vec{w}\right|+\left|\vec{v}-\vec{w}\right|

which gives \left|\vec{v}\right|-\left|\vec{w}\right|\leq\left|\vec{v}-\vec{w}\right|

Reversing the roles of \vec{v} and \vec{w} now gives the result: \bigl|\,\left|\vec{v}\right|-\left|\vec{w}\right|\,\bigr|\leq\left|\vec{v}-\vec{w}\right|

In the same manner, we can compare the directions of two different nonzero vectors \vec{v} and \vec{w} by either the direction of the difference $N(\vec{v} - \vec{w})$, or by the difference in the directions $N(\vec{v}) - N(\vec{w})$. Unlike the case for magnitudes, however, there is in general no relationship at all between these two measures of direction difference.

\def\vecf{{\vec{F}}}%\centerline{\hbox to 3.0in{\hrulefill}}

Now let us specialize this discussion of vectors to verification of wind vector data. We will denote the forecast wind vector by \vecf, and the observed wind vector by \veco. These are two-dimensional horizontal vectors with $u$ and $v$ components as follows:\vecf=(\uf,\vf)\hbox to1.0in{\hfill and\hfill}\veco=(\uo,\vo)

We will assume that we have $N$ observations of forecast and observed wind:

\vecfi=(\ufi,\vfi)\hbox to1.0in{\hfill and\hfill}\vecoi=(\uoi,\voi)

for $1 \leq i \leq N$. We also have the forecast and observed wind {\it speeds:}

$$

and, at each data point,

$$

It will be convenient to denote the average forecast and observed wind vectors by $\vecf_a$ and $\veco_a$:

$$

Now let us look at the definitions of the vector statistics produced by MET:

\def\mysep{\vskip 0.1in\centerline{\hbox to 5.0in{\hrulefill}}}

\mysep

FBAR and OBAR are the average values of the forecast and observed wind speed.

$$$$

\mysep

FS_RMS and OS_RMS are the root-mean-square values of the forecast and observed wind speeds.

$$

$$

\mysep

MSVE and RMSVE are, respectively, the mean squared, and root mean squared, lengths of the vector difference between the forecast and observed wind vectors.

$$$$

\mysep

FSTDEV and OSTDEV are the standard deviations of the forecast and observed wind speeds. In these equations, $\mu_f$ and $\mu_o$ are the average forecast and observed wind speeds

$$$$

\mysep

FDIR and ODIR are the direction (angle) of $\vecf_a$ and $\veco_a$ with respect to the grid directions.

$$$$

\mysep

FBAR_SPEED and OBAR_SPEED are the lengths of the average forecast and observed wind vectors. Note that this is {\it not} the same as the average forecast and observed wind speeds ({\it ie.,} the length of an average vector $\neq$ the average length of the vector).

$$$$

\mysep

VDIFF_SPEED is the length ({\it ie.~speed} of the vector difference between the average forecast and average observed wind vectors.

$$

Note that this is {\it not} the same as the difference in lengths (speeds) of the average forecast and observed wind vectors. That quantity is called SPEED_ERR (see below). There is a relationship between these two statistics however: using some of the results obtained in the introduction to this appendix, we can say that $\bigl|\,\left|\vecf_a\right| - \left|\veco_a\right|\,\bigr|\leq\left| \vecf_a - \veco_a  \right|$ or , equivalently, that $\left| \hbox{SPEED\_ERR} \right| \leq \hbox{VDIFF\_SPEED}$.

\mysep

VDIFF_DIR is the direction of the vector difference of the average forecast and average observed wind vectors. Note that this is {\it not} the same as the difference in direction of the average forecast and average observed wind vectors. This latter quantity would be $\hbox{FDIR} - \hbox{ODIR}$.

$$

\mysep

SPEED_ERR is the difference in the lengths (speeds) of the average forecast and average observed wind vectors. (See the discussion of VDIFF_SPEED above.)

$$

\mysep

SPEED_ABSERR is the absolute value of SPEED_ERR. Note that we have $\hbox{SPEED\_ABSERR} \leq \hbox{VDIFF\_SPEED}$ (see the discussion of VDIFF_SPEED above).

$$

\mysep

DIR_ERR is the signed angle between the directions of the average forecast and average observed wind vectors. Positive if the forecast vector is counterclockwise from the observed vector.

$$

\mysep

DIR_ABSERR is the absolute value of DIR_ERR. In other words, it's an unsigned angle rather than a signed angle.

$$

\mysep
