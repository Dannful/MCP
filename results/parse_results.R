library(tidyverse)
library(here)
library(ggplot2)

base_dir <- here::here("results/")

file_paths <- list.files(
  path = base_dir,
  pattern = "\\.out$",
  full.names = TRUE,
  recursive = TRUE
)

all_data <- list()

for (file_path in file_paths) {
  path_parts <- unlist(str_split(file_path, "/"))

  path_parts <- path_parts[path_parts != ""]

  start_index <- if (base_dir == ".") 2 else length(path_parts) - 4

  program_type <- path_parts[start_index]
  size <- path_parts[start_index + 1]
  num_tasks <- path_parts[start_index + 2]
  replication_index <- path_parts[start_index + 3]

  worker_number_with_ext <- path_parts[start_index + 4]
  worker_number <- str_remove(worker_number_with_ext, "\\.out$")

  df <- read_csv(file_path, col_types = cols(event = col_character(), seconds = col_double()))

  df <- df |>
    mutate(
      program_type = as.factor(program_type),
      size = as.numeric(size), # Convert to numeric if appropriate
      num_tasks = as.numeric(num_tasks), # Convert to numeric if appropriate
      replication_index = as.numeric(replication_index), # Convert to numeric
      worker_number = as.numeric(worker_number), # Convert to numeric
    ) |>
    rename(elapsed_seconds = seconds) # Rename seconds column

  all_data[[length(all_data) + 1]] <- df
}

dataset <- bind_rows(all_data)

# Take the mean of the replications
dataset <- dataset |>
  group_by(event, program_type, size, num_tasks, worker_number) |>
  summarise(elapsed_seconds = mean(elapsed_seconds))

# Take the mean of the workers
dataset <- dataset |>
  group_by(event, program_type, size, num_tasks) |>
  summarise(elapsed_seconds = mean(elapsed_seconds))

generate_plot <- function(df, path) {
  ggplot(data = df, mapping = aes(x = num_tasks, y = elapsed_seconds, color = program_type)) +
    geom_line() +
    facet_wrap(~size ~ event) +
    theme_minimal() -> plot
  ggsave(path, plot = plot)
}

generate_plot(dataset |> filter(size == 10), "./10.pdf")
generate_plot(dataset |> filter(size == 1000), "./1000.pdf")
generate_plot(dataset |> filter(size == 10000), "./10000.pdf")
