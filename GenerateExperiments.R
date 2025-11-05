library(DoE.base)
library(tidyverse)
generate_experiment <- function(factors, seed, path) {
    replications <- 3
    DoE.base::fac.design(nfactors = length(factors), replications = replications,
        repeat.only = FALSE, blocks = 1, randomize = TRUE, seed = seed, nlevels = sapply(factors,
            length), factor.names = factors) |>
        rename(Replication = Blocks) |>
        readr::write_csv(path)
}

seed <- 0
factors <- list(Type = c("Collective", "Blocking", "Non-blocking"), n = c(10, 1000,
    1e+06, 1e+09, 1e+12, 1e+15), Tasks = c(1, 2, 3, 4))
generate_experiment(factors, seed, "./experiments.csv")
