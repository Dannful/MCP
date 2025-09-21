library(DoE.base)
library(tidyverse)

generate_experiment <- function(factors, seed, path) {
  replications <- 3
  DoE.base::fac.design(
    nfactors = length(factors),
    replications = replications,
    repeat.only = FALSE,
    blocks = 1,
    randomize = TRUE,
    seed = seed,
    nlevels = sapply(factors, length),
    factor.names = factors
  ) |> readr::write_csv(path)
}

seed <- 0
factors <- list(
  Resolution = c("640 480", "3840 2160"),
  Otimization = c("collapse", "samples", "tasks"),
  Samples = c(30, 900),
  Threads = c(1, 20, 40)
)
generate_experiment(factors, seed, "./experiments.csv")
