% Generated by roxygen2 (4.0.1): do not edit by hand
\name{modelValuesSpec}
\alias{modelValuesSpec}
\title{Create the specs for a custom NIMBLE modelValues Object}
\usage{
modelValuesSpec(symTab, className, vars, types, sizes, modelDef = NA,
  where = globalenv())
}
\arguments{
  \item{vars}{A vector of character strings naming each
  variable in the modelValues object}

  \item{types}{A vector of character strings describing the
  type of data for the modelValues object. Options include
  `double' (for real-valued variables) and `int'.}

  \item{sizes}{A list in which the named items of the list
  match the \code{var} arguments and each item is a numeric
  vector of the dimensions}

  \item{symTab}{For internal use only}

  \item{className}{For internal use only}

  \item{where}{For internal use only}
}
\description{
Builds an R-based modelValues spec object
}
\details{
See the User Manual or \code{help(modelValuesBaseClass)}
and \code{help(modelValues)} for information
}
\examples{
#Custom modelValues object:
mvSpec <- modelValuesSpec(vars = c('x', 'y'),
				types = c('double', 'int'),
				sizes = list(x = 3, y = c(2,2)))
custom_mv <- modelValues(mvSpec, m = 2)
custom_mv['y',]
[[1]]
     [,1] [,2]
[1,]   NA   NA
[2,]   NA   NA

[[2]]
     [,1] [,2]
[1,]   NA   NA
[2,]   NA   NA
}
\author{
Clifford Anderson-Bergman
}

