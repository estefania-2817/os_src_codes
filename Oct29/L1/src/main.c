#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  FILE *file;
  FILE *output;
  double uptime, idle_time;
  int cpu_cores = 0; //counter for cpu cores
  char buffer[256];

  // Read /proc/uptime // contains two numbers: system uptime and total idle time
  file = fopen("/proc/uptime", "r");
  if (file == NULL) {
    printf("Error: Cannot open /proc/uptime\n");
    return 1;
  }

  fscanf(file, "%lf %lf", &uptime, &idle_time);
  fclose(file);

  // Read /proc/cpuinfo and count CPU cores
  file = fopen("/proc/cpuinfo", "r");
  if (file == NULL) {
    printf("Error: Cannot open /proc/cpuinfo\n");
    return 1;
  }

  //Looks for lines starting with "processor" Each "processor" entry represents one CPU core
  while (fgets(buffer, sizeof(buffer), file)) {
    if (strncmp(buffer, "processor", 9) == 0) {
      cpu_cores++;
    }
  }
  fclose(file);

  // Calculate CPU utilization percentage
  double utilization = (1.0 - (idle_time / (uptime * cpu_cores))) * 100.0;

  // Ensure utilization is within valid range (0% to 100%)
  if (utilization < 0.0)
    utilization = 0.0;
  if (utilization > 100.0)
    utilization = 100.0;

  // Convert uptime to human-readable format
  int total_seconds = (int)uptime;
  int days = total_seconds / (24 * 3600); //seconds in a day
  total_seconds %= (24 * 3600);  //remainder of seconds
  int hours = total_seconds / 3600;    // seconds in an hour
  total_seconds %= 3600; //remainder of seconds
  int minutes = total_seconds / 60;    //seconds in a minute
  int seconds = total_seconds %= 60; //remainder of seconds TOTAL

  // Generate report
  output = fopen("../report.txt", "w");
  if (output == NULL) {
    printf("Error: Cannot create report.txt\n");
    return 1;
  }

  fprintf(output, "--- System Activity Analysis ---\n");
  fprintf(output, "Detected CPU Cores: %d\n", cpu_cores);
  fprintf(output, "Total Uptime: %d days, %d hours, %d minutes, %d seconds\n",
          days, hours, minutes, seconds);
  fprintf(output, "Idle Time (Sum of Cores): %.2lf seconds\n", idle_time);
  fprintf(output, "Global CPU Utilization Percentage: %.2lf %%\n", utilization);
  fprintf(output, "---\n");

  fclose(output);

  // Display success message
  printf("[success]: The system analysis has been saved to the file "
         "'report.txt'.\n");

  return 0;
}