.. _appendixG:

Appendix G Vectors and Vector Statistics
========================================

In this appendix, we discuss some basic properties of vectors, concentrating on the two-dimensional case. To keep the discussion simple, we will assume we are using a Cartesian coordinate system.

Traditionally, vectors have been defined as quantities having both magnitude and direction, exemplified by a directed line segment. The magnitude of the vector is shown by the length of the segment, and the direction of the vector is usually shown by drawing an arrowhead on one end of the segment. Computers (and, in the author's experience, people) are usually more comfortable working with numbers, and so instead of the usual graphical definition of a vector, we will take the definition used in analytic geometry: A (two-dimensional) vector is an ordered pair of numbers. We will use boldface type to denote vectors, and so we can write 

.. math:: \mathbf{v} = (a,b)

to show that the vector :math:`\mathbf{v}` consists of the ordered pair of numbers *a* and *b*. The number *a* is called the first (or *x*) component of :math:`\mathbf{v}`, and *b* is called the second (or *y*) component. Vector addition is performed component-wise: :math:`(a, b) + (c, d) = (a + c, b + d)`, and similarly for subtraction. If :math:`\alpha` is a scalar, then we define multiplication by the scalar :math:`\alpha \text{ as } \alpha (a, b) = (\alpha a, \alpha b)`, and similarly for division by a (nonzero) scalar.

The *norm* (or length, or magnitude) of a vector :math:`\mathbf{v} = (a, b)`, is

.. math:: |\mathbf{v}| = \sqrt{a^{2} + b^{2}}

Note that :math:`|\mathbf{v} | = 0` if and only if :math:`a = b = 0`, in which case we say that :math:`\mathbf{v}` is the *zero vector*. If :math:`\alpha` is a scalar, then

.. math:: |\alpha\mathbf{v}| =|\alpha||\mathbf{v}|

The most important relation between vectors and their norms is given by the *triangle inequality*:

.. math:: |\mathbf{v} + \mathbf{w}| \leq | \mathbf{v}| + | \mathbf{w}|

In some cases, only the direction of a vector is of interest, and in such cases we can replace a nonzero vector :math:`\mathbf{v}` by the unique vector :math:`N(\mathbf{v})` that has the same direction as :math:`\mathbf{v}`, but has norm 1:

.. math:: N(\mathbf{v}) = \frac{\mathbf{v}}{| \mathbf{v}|}

The vector :math:`N(\mathbf{v})` will be called the *unit vector* corresponding to :math:`\mathbf{v}`, or more simply the *direction* of :math:`\mathbf{v}`. Note that the zero vector has no direction.

Since vectors are characterized by magnitude (norm) and direction, this gives two ways to compare vectors: we can compare either their magnitudes or their directions. If :math:`\mathbf{v} \text{ and } \mathbf{w}` are vectors, then we can compare their norms by either taking the norm of the difference :math:`| \mathbf{v} - \mathbf{w}|` or the difference of the norms :math:`| \mathbf{v}| - |\mathbf{w}|`. It's not always made clear in verification studies which of these is meant, and in general these two quantities will be different. However, by making use of the triangle inequality it can be shown that there is a relation between them. To derive this, let :math:`\mathbf{z} = \mathbf{v} - \mathbf{w}`, from which we get :math:`\mathbf{v} = \mathbf{w} + \mathbf{z}`. Now taking norms and using the triangle inequality,

.. math:: | \mathbf{v}| = | \mathbf{w} + \mathbf{z} | \leq | \mathbf{w}| + | \mathbf{z} | = | \mathbf{w} | + | \mathbf{v} - \mathbf{w}|

which gives

.. math:: | \mathbf{v}| - | \mathbf{w}| \leq | \mathbf{v} - \mathbf{w}|

Reversing the roles of :math:`\mathbf{v} \text{ and } \mathbf{w}` now gives the result:

.. math:: | | \mathbf{v}| - | \mathbf{w}|| \leq | \mathbf{v} - \mathbf{w}|

In the same manner, we can compare the directions of two different nonzero vectors :math:`\mathbf{v} \text{ and } \mathbf{w}` by either the direction of the difference :math:`N(\mathbf{v} - \mathbf{w})`, or by the difference in the directions :math:`N(\mathbf{v}) - N(\mathbf{w})`. Unlike the case for magnitudes, however, there is in general no relationship at all between these two measures of direction difference.

__________________________


Now let us specialize this discussion of vectors to verification of wind vector data. We will denote the forecast wind vector by :math:`\mathbf{F}`, and the observed wind vector by **O**. These are two-dimensional horizontal vectors with *u* and *v* components as follows:

.. math:: \mathbf{F} = (u_f,v_f) \text{ and } \mathbf{O} = (u_o,v_o)

We will assume that we have *N* observations of forecast and observed wind:

.. math:: \mathbf{F}_i = (u_{fi},v_{fi}) \text{ and } \mathbf{O}_i = (u_{oi},v_{oi})

for :math:`1 \leq i \leq N`. We also have the forecast and observed wind *speeds*:

.. math:: s_f = | \mathbf{F} | = \sqrt{u_f^2 + v_f^2} \text{ and } s_o = | \mathbf{O} | = \sqrt{u_o^2 + v_o^2}

and, at each data point,

.. math:: s_{fi} = | \mathbf{F}_i | = \sqrt{u_{fi}^2 + v_{fi}^2} \text{ and } s_{oi} = | \mathbf{O}_i | = \sqrt{u_{oi}^2 + v_{oi}^2}

It will be convenient to denote the average forecast and observed wind vectors by :math:`\mathbf{F}_a \text{ and } \mathbf{O}_a`:

.. math:: \mathbf{F}_a = \frac{1}{N} \sum_i \mathbf{F}_i \text{ and } \mathbf{O}_a = \frac{1}{N} \sum_i \mathbf{O}_i

Now let us look at the definitions of the vector statistics produced by MET:

_________________________


FBAR and OBAR are the average values of the forecast and observed wind speed.

.. math:: \text{FBAR} = \frac{1}{N} \sum_i s_{fi}
	  
	  \text{OBAR} = {1 \over N} \sum_i s_{oi}

_________________________


FS_RMS and OS_RMS are the root-mean-square values of the forecast and observed wind speeds.

.. math:: \text{FS_RMS} = [ \frac{1}{N} \sum_i s_{fi}^2]^{1/2}

 \text{OS_RMS} = [\frac{1}{N} \sum_i s_{oi}^2]^{1/2}

___________________________

MSVE and RMSVE are, respectively, the mean squared, and root mean squared, lengths of the vector difference between the forecast and observed wind vectors.

.. math:: \text{MSVE} = \frac{1}{N} \sum_i | \mathbf{F}_i - \mathbf{O}_i|^2

	  \text{RMSVE} = \sqrt{MSVE}

____________________________


FSTDEV and OSTDEV are the standard deviations of the forecast and observed wind speeds.

.. math:: \text{FSTDEV } = \frac{1}{N} \sum_i (s_{fi} - \text{FBAR})^2 = \frac{1}{N} \sum_i s_{fi}^2 - \text{FBAR}^2

 \text{OSTDEV } = \frac{1}{N} \sum_i (s_{oi} - \text{OBAR})^2 = \frac{1}{N} \sum_i s_{oi}^2 - \text{OBAR}^2 

___________________________

FDIR and ODIR are the direction (angle) of :math:`\mathbf{F}_a \text{ and } \mathbf{O}_a` with respect to the grid directions.

.. math:: \text{FDIR } = \text{ direction angle of } \mathbf{F}_a
	  
	  \text{ODIR} = \text{ direction angle of } \mathbf{O}_a

________________________


FBAR_SPEED and OBAR_SPEED are the lengths of the average forecast and observed wind vectors. Note that this is *not* the same as the average forecast and observed wind speeds (*ie.,* the length of an average vector :math:`\neq` the average length of the vector).

.. math:: \text{FBAR_SPEED } = | \mathbf{F}_a |
	  
	  \text{OBAR_SPEED } = | \mathbf{O}_a |

________________________


VDIFF_SPEED is the length (*ie. speed*) of the vector difference between the average forecast and average observed wind vectors.

.. math:: \text{VDIFF_SPEED } = | \mathbf{F}_a - \mathbf{O}_a |

Note that this is *not* the same as the difference in lengths (speeds) of the average forecast and observed wind vectors. That quantity is called SPEED_ERR (see below). There is a relationship between these two statistics however: using some of the results obtained in the introduction to this appendix, we can say that :math:`| | \mathbf{F}_a | - | \mathbf{O}_a | | \leq | \mathbf{F}_a - \mathbf{O}_a |` or , equivalently, that :math:`| \text{SPEED_ERR} | \leq \text{VDIFF_SPEED}`.

_________________________


VDIFF_DIR is the direction of the vector difference of the average forecast and average observed wind vectors. Note that this is {\it not} the same as the difference in direction of the average forecast and average observed wind vectors. This latter quantity would be FDIR :math:`-` ODIR.

.. math:: \text{VDIFF_DIR } = \text{ direction of } (\mathbf{F}_a - \mathbf{O}_a)

_________________________


SPEED_ERR is the difference in the lengths (speeds) of the average forecast and average observed wind vectors. (See the discussion of VDIFF_SPEED above.)

.. math:: \text{SPEED_ERR } = | \mathbf{F}_a | - | \mathbf{O}_a | = \text{ FBAR_SPEED } - \text{ OBAR_SPEED}

___________________________


SPEED_ABSERR is the absolute value of SPEED_ERR. Note that we have SPEED_ABSERR :math:`\leq` VDIFF_SPEED (see the discussion of VDIFF_SPEED above).

.. math:: \text{SPEED_ABSERR } = | \text{SPEED_ERR} |

__________________________

DIR_ERR is the signed angle between the directions of the average forecast and average observed wind vectors. Positive if the forecast vector is counterclockwise from the observed vector.

.. math:: \text{DIR_ERR } = \text{ direction between } N(\mathbf{F}_a) \text{ and } N(\mathbf{O}_a) 

__________________________
	  
DIR_ABSERR is the absolute value of DIR_ERR. In other words, it's an unsigned angle rather than a signed angle.

.. math:: \text{DIR_ABSERR } = | \text{DIR_ERR}|
