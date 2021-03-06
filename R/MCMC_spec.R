


controlDefaultList <- list(
    adaptive = TRUE,
    adaptScaleOnly = FALSE,
    adaptInterval = 200,
    scale = 1,
    propCov = 'identity',
    sliceWidth = 1,
    sliceMaxSteps = 100
)



samplerSpec <- setRefClass(
    Class = 'samplerSpec',
    fields = list(
        type    = 'ANY',
        control = 'ANY'),
    methods = list(
    	initialize =function(...) {
            control <<- list()
            callSuper(...)
        },
        buildSampler = function(model, mvSaved) {
            samplerNfName <- paste0('sampler_', type)
            eval(call(samplerNfName, model=model, mvSaved=mvSaved, control=control))
        },
        toStr = function() {
            return(paste0(type, ' sampler;   ', mcmc_listContentsToStr(control)))
        }
    )
)


# NOTE: methods are documented as a "docstring" with each method - see 'removeSamplers' below. roxygen will automatically grab info from these docstrings and inject into the Rd in the Methods Section
# NOTE: including the name of the class in @aliases is important because by default we only get help("MCMCspec-class") and not help(MCMCspec)
# NOTE: the empty lines are important in the final formatting, so please don't remove any of them in your own help info

#' Class \code{MCMCspec}
#' @aliases MCMCspec configureMCMC addSampler removeSamplers setSamplers getSamplers addMonitors addMonitors2 resetMonitors getMonitors setThin setThin2
#' @export
#' @description
#' Objects of this class fully specify an MCMC algorithm, specific to a particular model.
#' Given an object spec of class MCMCspec, the actual MCMC function may subsequently be built by calling buildMCMC(spec).
#' See documentation for method initialize() or configureMCMC(), for details of creating an MCMCspec object.
#' @author Daniel Turek
#' @examples
#' code <- nimbleCode({
#'  mu ~ dnorm(0, 1)
#'  x ~ dnorm(mu, 1)
#' })
#' Rmodel <- nimbleModel(code)
#' spec <- configureMCMC(Rmodel)
#' spec$setSamplers(1)
#' spec$addSampler(type = 'slice', control = list(targetNode = 'x'))
#' spec$addMonitors('mu', thin = 1)
#' spec$addMonitors2('x', thin2 = 10)
#' spec$getMonitors()
#' spec$getSamplers()

MCMCspec <- setRefClass(
    
    Class = 'MCMCspec',                           
    
    fields = list(
        model               = 'ANY',
        monitors            = 'ANY', 		#'character',
        monitors2           = 'ANY', 		#'character',
        thin                = 'ANY', 		#'numeric',
        thin2               = 'ANY', 		#'numeric',
        samplerSpecs        = 'ANY', 		#'list',
        controlDefaults     = 'ANY', 		#'list',
        controlNamesLibrary = 'ANY', 		#'list'
        mvSamples1Spec 		= 'ANY',
        mvSamples2Spec		= 'ANY'
    ),
    
    methods = list(
        
        initialize = function(model, nodes, control = list(),
                              monitors,                thin  = 1,
                              monitors2 = character(), thin2 = 1,
                              useConjugacy = TRUE, onlyRW = FALSE, onlySlice = FALSE, multivariateNodesAsScalars = FALSE,
                              print = FALSE) {	
'	
Creates a defaut MCMC specification for a given model.  The resulting object is suitable as an argument to buildMCMC().

Arguments:

model: A NIMBLE model object, created from nimbleModel(...)

nodes: An optional character vector, specifying the nodes for which samplers should be created.
Nodes may be specified in their indexed form, \'y[1, 3]\', or nodes specified without indexing will be expanded fully, e.g., \'x\' will be expanded to \'x[1]\', \'x[2]\', etc.
If missing, the default value is all non-data stochastic nodes.
If NULL, then no samplers are added.

control: An optional list of control arguments to sampler functions.  If a control list is provided, the elements will be provided to all sampler functions which utilize the named elements given.
For example, the standard Metropolis-Hastings random walk sampler (sampler_RW) utilizes control list elements \'adaptive\', \'adaptInterval\', \'scale\', 
and also \'targetNode\' however this should not generally be provided as a control list element to configureMCMC().
The default values for control list arguments for samplers (if not otherwise provided as an argument to configureMCMC() ) are contained in the \'controlDefaultList\' object.

monitors: A character vector of node names or variable names, to record during MCMC sampling.
This set of monitors will be recorded with thinning interval \'thin\', and the samples will be stored into the \'mvSamples\' object.
The default value is all top-level stochastic nodes of the model -- those having no stochastic parent nodes.

monitors2: A character vector of node names or variable names, to record during MCMC sampling.
This set of monitors will be recorded with thinning interval \'thin2\', and the samples will be stored into the \'mvSamples2\' object.
The default value is an empty character vector, i.e. no values will be recorded.

thin: The thinning interval for \'monitors\'.  Default value is one.

thin2: The thinning interval for \'monitors2\'.  Default value is one.

useConjugacy: A boolean argument, with default value TRUE.  If specified as FALSE, then no conjugate samplers will be used, even when a node is determined to be in a conjugate relationship.

onlyRW: A boolean argument, with default value FALSE.  If specified as TRUE, then Metropolis-Hastings random walk samplers (sampler_RW) will be assigned for all non-terminal continuous-valued nodes nodes.
Discrete-valued nodes are assigned a slice sampler (sampler_slice), and terminal (predictive) nodes are assigned an end sampler (sampler_end).

onlySlice: A boolean argument, with default value FALSE.  If specified as TRUE, then a slice sampler is assigned for all non-terminal nodes.
Terminal (predictive) nodes are still assigned an end sampler (sampler_end).

multivariateNodesAsScalars: A boolean argument, with default value FALSE.  If specified as TRUE, then non-terminal multivariate stochastic nodes will have scalar samplers assigned to each of the scalar components of the multivariate node.  The default value of FALSE results in a single block sampler assigned to the entire multivariate node.  Note, multivariate nodes appearing in conjugate relationships will be assigned the corresponding conjugate sampler (provided useConjugacy == TRUE), regardless of the value of this argument.

print: Boolean argument, specifying whether to print the ordered list of default samplers.
'
            
            samplerSpecs <<- list(); controlDefaults <<- list(); controlNamesLibrary <<- list(); monitors <<- character(); monitors2 <<- character();
            model <<- model
            addMonitors( monitors,  print = FALSE)
            addMonitors2(monitors2, print = FALSE)
            thin  <<- thin
            thin2 <<- thin2
            samplerSpecs    <<- list()
            controlDefaults <<- controlDefaultList
            for(i in seq_along(control))     controlDefaults[[names(control)[i]]] <<- control[[i]]
            controlNamesLibrary <<- list()
            
            if(identical(nodes, character())) { nodes <- model$getNodeNames(stochOnly = TRUE, includeData = FALSE)
            } else             { if(is.null(nodes) || length(nodes)==0)     nodes <- character(0)
                                 nl_checkVarNamesInModel(model, removeIndexing(nodes))
                                 nodes <- model$expandNodeNames(nodes)            }
        
            nodes <- model$topologicallySortNodes(nodes)   ## topological sort
            isNodeEnd <- model$isNodeEnd(nodes)
            #   nodes %in% model$getMaps('nodeNamesEnd')
        
            for(i in seq_along(nodes) ) {
            	node <- nodes[i]
                discrete <- model$getNodeInfo()[[node]]$isDiscrete()
                nodeScalarComponents <- model$expandNodeNames(node, returnScalarComponents = TRUE)
                nodeLength <- length(nodeScalarComponents)
                
                ## if node has 0 stochastic dependents, assign 'end' sampler (e.g. for predictive nodes)
             	if(isNodeEnd[i]) { addSampler(type = 'end', control = list(targetNode=node), print = print);     next }
                
                ## for multivariate nodes, either add a conjugate sampler, or RW_block sampler
                if(nodeLength > 1) {
                    if(useConjugacy) {
                        conjugacyResult <- model$checkConjugacy(node)
                        if(!is.null(conjugacyResult)) {
                            addSampler(type = conjugacyResult$samplerType, control = conjugacyResult$control, print = print);     next }
                    }
                    if(multivariateNodesAsScalars) {
                        for(scalarNode in nodeScalarComponents) {
                            addSampler(type = 'RW', control = list(targetNode=scalarNode), print = print) };     next }
                    addSampler(type = 'RW_block', control = list(targetNodes=node), print = print);     next }

                ## node is scalar, non-end node
                if(onlyRW && !discrete)   { addSampler(type = 'RW',    control = list(targetNode=node), print = print);     next }
                if(onlySlice)             { addSampler(type = 'slice', control = list(targetNode=node), print = print);     next }
                
                ## if node passes checkConjugacy(), assign 'conjugate_dxxx' sampler
                if(useConjugacy) {
                    conjugacyResult <- model$checkConjugacy(node)
                    if(!is.null(conjugacyResult)) {
                        addSampler(type = conjugacyResult$samplerType, control = conjugacyResult$control, print = print);     next }
                }
                
                ## if node distribution is discrete, assign 'slice' sampler
                if(discrete) { addSampler(type = 'slice', control = list(targetNode=node), print = print);     next }
                
                ## default: 'RW' sampler
                addSampler(type = 'RW', control = list(targetNode=node), print = print);     next
            }
},
        
        addSampler = function(type, control = list(), print = TRUE) {
'
Adds a sampler to the list of samplers contained in the MCMCspec object.

Arguments:

type: The type of sampler to add.  If type=\'newSamplerType\', then sampler_newSamplertype must correspond to a nimbleFunction generator.  Otherwise an error results.

control: A list of control arguments for sampler_newSamplertype.
These will override the defaults contained in the \'controlDefaultList\' object, and any specified in the control list argument to configureMCMC().
An error results if sampler_newSamplertype requires any control elements which are 
not present in this argument, the control list argument to configureMCMC(), or in the \'controlDefaultList\' object.

print: Boolean argument, specifying whether to print the details of the newly added sampler, as well as its position in the list of MCMC samplers.

Details: A single instance of the newly specified sampler is added to the end of the list of samplers for this MCMCspec object.
'
            samplerFunctionName <- as.name(paste0('sampler_', type))
            if(inherits(try(eval(samplerFunctionName), silent=TRUE), 'try-error'))     stop(paste0('No function definition found for sampler type: ', samplerFunctionName))  ## sampler function doesn't exist
            if(is.null(controlNamesLibrary[[type]]))     controlNamesLibrary[[type]] <<- mcmc_findControlListNamesInCode(eval(samplerFunctionName))   ## populate the library of names for the control list
            controlListNames <- controlNamesLibrary[[type]]
            thisControlList <- controlDefaults           ## start with all the defaults
            thisControlList[names(control)] <- control   ## add in any controls provided as an argument
            if(!all(controlListNames %in% names(thisControlList)))  stop(paste0('Required control names are missing for ', samplerFunctionName, ': ', paste0(setdiff(controlListNames, names(thisControlList)), collapse=', ')))
            if(!all(names(control) %in% controlListNames))   warning(paste0('Superfluous control names were provided for ', samplerFunctionName, ': ', paste0(setdiff(names(control), controlListNames), collapse=', ')))
            thisControlList <- thisControlList[controlListNames]
            newSamplerInd <- length(samplerSpecs) + 1
            samplerSpecs[[newSamplerInd]] <<- samplerSpec(type=type, control=thisControlList)
            if(print) getSamplers(newSamplerInd)
        },
        
        removeSamplers = function(ind, print = TRUE) {
'
Removes one or more samplers from an MCMCspec object.

Arguments:

ind: A numeric vector, giving the indices of the samplers to be removed.  If omitted, then all samplers are removed.

print: Boolean argument, default value TRUE, specifying whether to print the current list of samplers once the removal has been done.
'      
            if(missing(ind))   ind <- seq_along(samplerSpecs)
            samplerSpecs[ind] <<- NULL
            if(print) getSamplers()
            return(invisible(NULL))
        },
        
        setSamplers = function(ind, print = TRUE) {
            '
Sets the ordering of the list of MCMC samplers.

Arguments:

ind: A numeric vector, specifying the new list of MCMC samplers, in terms of the current ordered list of samplers.
For example, if the MCMCspec object currently has 3 samplers, then the ordering may be reversed by calling mcmcspec$setSamplers(3:1),
the list may be changed to only calling the first sampler 3 times, then the remaining two samplers by calling mcmcspec$setSamplers(c(1, 1, 1, 2, 3)),
or all samplers may be removed by calling mcmcspec$setSamplers(numeric(0)).

print: Boolean argument, default value TRUE, specifying whether to print the new list of samplers.
'   
            if(missing(ind))   ind <- numeric(0)
            samplerSpecs <<- samplerSpecs[ind]
            if(print) getSamplers()
            return(invisible(NULL))
        },
        
        getSamplers = function(ind) {
'
Prints details of the MCMC samplers.

Arguments:

ind: A numeric vector, specifying the indices of the samplers to print.  If omitted, then all samplers are printed.
This is generally the intended usage, to see all current samplers in the MCMCspec object.
'  
            if(missing(ind))     ind <- seq_along(samplerSpecs)
            for(i in ind) {
                cat(paste0('[', i, '] ', samplerSpecs[[i]]$toStr(), '\n'))
            }
        },
        
        addMonitors = function(vars, ind = 1, print = TRUE) {
            '
Adds variables to the list of monitors.

Arguments:

vars: A character vector of indexed nodes, or variables, which are to be monitored.  These are added onto the current monitors list.

print: A boolean variable, specifying whether to print all current monitors.

Details: See the initialize() function
            '
            
            if(isMvSamplesReady(ind)){
            	cat('Changing monitors, even though an MCMC has been built already. When compiling the MCMC, use resetFunctions = TRUE option\n')
            	if(ind == 1)
            		mvSamples1Spec <<- NULL
            	if(ind == 2)
            		mvSamples2Spec <<- NULL
            }
            
            
            
            if(is.null(vars))  vars <- model$getNodeNames(topOnly = TRUE, stochOnly = TRUE)
            vars <- unique(removeIndexing(vars))
            nl_checkVarNamesInModel(model, vars)
            if(ind == 1)     monitors  <<- unique(c(monitors,  vars))
            if(ind == 2)     monitors2 <<- unique(c(monitors2, vars))
            if(print) getMonitors()
            return(invisible(NULL))
        },

        addMonitors2 = function(vars, print = TRUE) {
    '
Adds variables to the list of monitors2.

Arguments:

vars: A character vector of indexed nodes, or variables, which are to be monitored.  These are added onto the current monitors2 list.

print: A boolean variable, specifying whether to print all current monitors.

Details: See the initialize() function
            '
    addMonitors(vars, ind = 2, print = print)
},
        
        resetMonitors = function() {
            '
Resets the current monitors and monitors2 lists to nothing.

Details: See the initialize() function
            '
            monitors  <<- character()
            monitors2 <<- character()
            
            if(isMvSamplesReady(1) || isMvSamplesReady(2)){
            	cat('Changing monitors, even though an MCMC has been built already. When compiling the MCMC, use resetFunctions = TRUE option\n')
            	mvSamples1Spec <<- NULL
            	mvSamples2Spec <<- NULL
            }

            
            return(invisible(NULL))
        },
        
        getMonitors = function() {
            '
Prints all current monitors and monitors2

Details: See the initialize() function
            '
            if(length(monitors)  > 0)   cat(paste0('thin = ', thin,  ': ', paste0(monitors,  collapse = ', '), '\n'))
            if(length(monitors2) > 0)   cat(paste0('thin2 = ', thin2, ': ', paste0(monitors2, collapse = ', '), '\n'))
        },
        
        setThin  = function(thin, print = TRUE) {
            '
Sets the value of thin.

Arguments:

thin: The new value for the thinning interval \'thin\'.

print: A boolean variable, specifying whether to print all current monitors.

Details: See the initialize() function
            '
            thin  <<- thin
            if(print) getMonitors()
            return(invisible(NULL))
        },
        setThin2 = function(thin2, print = TRUE) {
            '
Sets the value of thin2.

Arguments:

thin2: The new value for the thinning interval \'thin2\'.

print: A boolean variable, specifying whether to print all current monitors.

Details: See the initialize() function
            '
            thin2 <<- thin2
            if(print) getMonitors()
            return(invisible(NULL))
        },
        
        getMvSamplesSpec  = function(ind = 1){
        	
             if(isMvSamplesReady(ind) == TRUE) {
             		if(ind == 1) return(mvSamples1Spec)
             		return(mvSamples2Spec)
             	}
             	else{
             		makeMvSamplesSpec(ind)
             		if(ind == 1)
             			output <- mvSamples1Spec
             		if(ind == 2)
             			output <- mvSamples2Spec
             		return(output)
             		}
        },
        
        isMvSamplesReady = function(ind){
        	if(ind == 1) return(is(mvSamples1Spec, 'function'))		#Probably really want to give mvSpecs there own class...
        	if(ind == 2) return(is(mvSamples2Spec, 'function'))
        	stop('invalid indicator for isMvSsamplesReady')
        },
        
        makeMvSamplesSpec = function(ind){
			modelSymbolObjects = model$getSymbolTable()$getSymbolObjects()
			if(ind == 1) monitorNames = monitors
			if(ind == 2) monitorNames = monitors2
			if(!all(monitorNames %in% names(modelSymbolObjects))) stop('some monitor names are not in the model symbol table; this should never occur')
			thisModelValuesSpec = modelValuesSpec(symbolTable(symbols = modelSymbolObjects[monitorNames]))
			if(ind == 1) mvSamples1Spec <<- thisModelValuesSpec
			if(ind == 2) mvSamples2Spec <<- thisModelValuesSpec     	
        }
        
#        internal_newMv = function(ind) {
#            modelSymbolObjects <- model$getSymbolTable()$getSymbolObjects()
#            if(ind == 1)   monitorNames <- monitors
#            if(ind == 2)   monitorNames <- monitors2
#            if(!all(monitorNames %in% names(modelSymbolObjects))) stop('some monitor names are not in the model symbol table; this should never occur')
#            mv <- modelValues(symbolTable(symbols = modelSymbolObjects[monitorNames]))
#            return(mv)
#        }
    )
)



#' Turn BUGS model code into an object for use in \code{nimbleModel} or \code{readBUGSmodel}
#'
#' Simply keeps model code as an R call object, the form needed by \code{nimbleModel} and optionally usable by \code{readBUGSmodel}
#' 
#' @param code expression providing the code for the model 
#' @author Daniel Turek
#' @export
#' @details It is equivalent to use the R function \code{quote}.  \code{nimbleCode} is simply provided as a more readable alternative for NIMBLE users not familiar with \code{quote}.
#' @examples
#' code <- nimbleCode({
#'     x ~ dnorm(mu, sd = 1)
#'     mu ~ dnorm(0, sd = prior_sd)
#' })




#' Build the MCMCspec object for construction of an MCMC object
#'
#' Creates a defaut MCMC specification for a given model.  The resulting object is suitable as an argument to buildMCMC(). 
#'
#'@param model A NIMBLE model object, created from nimbleModel(...)
#'@param nodes An optional character vector, specifying the nodes for which samplers should be created.
#'Nodes may be specified in their indexed form, \'y[1, 3]\', or nodes specified without indexing will be expanded fully, e.g., \'x\' will be expanded to \'x[1]\', \'x[2]\', etc.
#'If missing, the default value is all non-data stochastic nodes.
#'If NULL, then no samplers are added.
#'@param control An optional list of control arguments to sampler functions.  If a control list is provided, the elements will be provided to all sampler functions which utilize the named elements given.
#'For example, the standard Metropolis-Hastings random walk sampler (sampler_RW) utilizes control list elements \'adaptive\', \'adaptInterval\', \'scale\', 
#'and also \'targetNode\' however this should not generally be provided as a control list element to configureMCMC().
#'The default values for control list arguments for samplers (if not otherwise provided as an argument to configureMCMC() ) are contained in the \'controlDefaultList\' object.
#'@param monitors A character vector of node names or variable names, to record during MCMC sampling.
#'This set of monitors will be recorded with thinning interval \'thin\', and the samples will be stored into the \'mvSamples\' object.
#'The default value is all top-level stochastic nodes of the model -- those having no stochastic parent nodes.
#'@param monitors2 A character vector of node names or variable names, to record during MCMC sampling.
#'This set of monitors will be recorded with thinning interval \'thin2\', and the samples will be stored into the \'mvSamples2\' object.
#'The default value is an empty character vector, i.e. no values will be recorded.
#'@param thin The thinning interval for \'monitors\'.  Default value is one.
#'@param thin2 The thinning interval for \'monitors2\'.  Default value is one.
#'@param useConjugacy A boolean argument, with default value TRUE.  If specified as FALSE, then no conjugate samplers will be used, even when a node is determined to be in a conjugate relationship.
#'@param onlyRW A boolean argument, with default value FALSE.  If specified as TRUE, then Metropolis-Hastings random walk samplers (sampler_RW) will be assigned for all non-terminal continuous-valued nodes nodes.
#'Discrete-valued nodes are assigned a slice sampler (sampler_slice), and terminal (predictive) nodes are assigned an end sampler (sampler_end).
#'@param onlySlice A boolean argument, with default value FALSE.  If specified as TRUE, then a slice sampler is assigned for all non-terminal nodes.
#'Terminal (predictive) nodes are still assigned an end sampler (sampler_end).
#'@param multivariateNodesAsScalars: A boolean argument, with default value FALSE.  If specified as TRUE, then non-terminal multivariate stochastic nodes will have scalar samplers assigned to each of the scalar components of the multivariate node.  The default value of FALSE results in a single block sampler assigned to the entire multivariate node.  Note, multivariate nodes appearing in conjugate relationships will be assigned the corresponding conjugate sampler (provided useConjugacy == TRUE), regardless of the value of this argument.
#'@param print Boolean argument, specifying whether to print the ordered list of default samplers.
#'@author Daniel Turek
#'@details See \code{MCMCspec} for details on how to manipulate the \code{MCMCspec} object
configureMCMC <- function(model, oldSpec, nodes, control = list(), 
						  monitors, thin = 1, monitors2 = character(), thin2=1,
						  useConjugacy = TRUE, onlyRW = FALSE, onlySlice = FALSE, multivariateNodesAsScalars = FALSE,
						  print = FALSE){
						  	if(!missing(oldSpec)){
						  		if(!is(oldSpec, 'MCMCspec'))
						  			stop('oldSpec must be an MCMCspec object, as built by the configureMCMC function')
						  		return(makeNewSpecFromOldSpec(oldSpec))	
						  	}
						  	
						  if(missing(model))
						  	stop('Either oldSpec or model must be supplied')
						  if(missing(nodes))
						  	nodes <- character()
						  if(missing(monitors))
						  	monitors <- NULL
						  
						  thisSpec <- MCMCspec(model = model, nodes = nodes, control = control, 
						  					monitors = monitors, thin = thin, monitors2 = monitors2, thin2 = thin2,
						  					useConjugacy = useConjugacy, onlyRW = onlyRW, onlySlice = onlySlice,
						  					multivariateNodesAsScalars = multivariateNodesAsScalars, print = print)
						  return(thisSpec)	
						  }



# This is function which builds a new MCMCspec from an old MCMCspec
# This is required to be able to a new C-based MCMC without recompiling
makeNewSpecFromOldSpec <- function(oldMCMCspec){
	newMCMCspec <- configureMCMC(oldMCMCspec$model, nodes = NULL)
	newMCMCspec$monitors <- oldMCMCspec$monitors
	newMCMCspec$monitors2 <- oldMCMCspec$monitors2
	newMCMCspec$thin <- oldMCMCspec$thin
	newMCMCspec$thin2 <- oldMCMCspec$thin2
	newMCMCspec$samplerSpecs <- oldMCMCspec$samplerSpecs
	newMCMCspec$controlDefaults <- oldMCMCspec$controlDefaults
	newMCMCspec$controlNamesLibrary <- oldMCMCspec$controlNamesLibrary
	newMCMCspec$mvSamples1Spec <- oldMCMCspec$mvSamples1Spec
	newMCMCspec$mvSamples2Spec <- oldMCMCspec$mvSamples2Spec
	return(newMCMCspec)	
}




##### things (from v0.1) for dealing with samplerOrder:
#
# getSamplers <- function(mcmcspec, print.all = FALSE) {
#     if(print.all) return(mcmcspec$samplerSpecs)
#     return(mcmcspec$samplerSpecs[mcmcspec$samplerOrder])
# }
# 
# setSamplerOrder <- function(mcmcspec, samplerOrder) {
#     mcmcspec$samplerOrder <- mcmcspec$samplerOrder[samplerOrder]
# }



