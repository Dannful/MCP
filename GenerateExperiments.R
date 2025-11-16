library(DoE.base)
library(tidyverse)
generate_experiment <- function(factors, seed, path) {
    replications <- 3
    options(scipen = 999)
    DoE.base::fac.design(nfactors = length(factors), replications = replications,
        repeat.only = FALSE, blocks = 1, randomize = TRUE, seed = seed, nlevels = sapply(factors,
            length), factor.names = factors) |>
        rename(Replication = Blocks) |>
        readr::write_csv(path)
}

seed <- 0
factors <- list(Type = c("mpi_coletiva", "mpi_p2p_bloqueante", "mpi_p2p_naobloqueante"),
    n = c(10, 1000, 1e+06, 1e+09, 1e+12, 1e+15), Tasks = c(4, 16, 32, 64, 80))
generate_experiment(factors, seed, "./experiments.csv")
