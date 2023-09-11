# Table of Contents

<!--TOC-->

# yamal-daemon

The **yamal-daemon** utility loads ytp files into memory and preemptively allocates pages in order to maximize write performance.

```bash
yamal-daemon -c <path> -s <section>
```

Where

* *-c \<path\>, \--config \<path\>*: configuration file path
* *-s, \--section \<section\>*: main section to be used for the tool configuration

The configuration file for two YTPs looks like:

```dosini
[main]
ytps=ytp1,ytp2,

[ytp1]
path="strg.ytp"
rate=0
initial_size=32

[ytp2]
path="symb.ytp"
rate=0
initial_size=250
```

Where

* *rate*: specifies the writer speed in bytes/ns. The tool will detect this by sampling the size of the reserved size frequently.
* *initial\_size*: it will preallocate the specified size in MB.

# yamal-perf

The **yamal-perf** utility measures bandwidth and latency of a YTP file. It writes and reads ytp messages from a YTP file and provides statistics via stdout.

The sink mode will read the messages from the YTP file and it will provide statistics via stdout every 1s.

```bash
yamal-perf sink [-x <priority>] [-a <cpuid>] [-i <time_ms>] [-c <string>] -f <path>
```

Where

* *-x \<priority\>, \--priority \<priority\>*: set the priority of the main process (1-99)
* *-a \<cpuid\>, \--affinity \<cpuid\>*: set the CPU affinity of the main process
* *-i \<time_ms\>, \--interval \<time_ms\>*: statistics interval in ms
* *-c \<string\>, \--channel \<string\>*: name of the channel to use
* *-f \<path\>, \--file \<path\>*: (required) ytp file to read the messages

Only -f is required

The source mode will write the messages to the YTP file.

```bash
yamal-perf source [-x <priority>] [-a <cpuid>] [-i <time_ms>] [-r <messages>] [-s <bytes>] [-m <integer>] [-c <string>] -f <path>
```

Where

* *-x \<priority\>, \--priority \<priority\>*: set the priority of the main process (1-99)
* *-a \<cpuid\>, \--affinity \<cpuid\>*: set the CPU affinity of the main process
* *-i \<time_ms\>, \--interval \<time_ms\>*: statistics interval in ms
* *-r \<messages\>, \--rate \<messages\>*: number of messages to publish in one second
* *-s \<bytes\>, \--size \<bytes\>*: size of the messages to publish
* *-m \<integer\>, \--messages \<integer\>*: number of messages to publish
* *-c \<string\>, \--channel \<string\>*: name of the channel to use
* *-f \<path\>, \--file \<path\>*: (required) ytp file to read the messages

Only -f is required.

Not providing -r or -m, it will publish infinite number of messages as fast as possible.

# yamal-run

The **yamal-run** utility enables users to load yamal components and execute them with the desired configuration.

```bash
yamal-run [-x <priority>] [-a <cpuid>] [-k] [-o <component>] [-m <module>] -c <config_path> -s <section> [--] [--version] [-h]
```

Where

* *-c \<config_path\>*: configuration file path
* *-s*: main section to be used for the tool configuration
* *-x \<priority\>*: optionally specify the desired priority
* *-a \<cpuid\>*: optionally specify the desired cpu affinity
* *-o \<component\>*: component name. Must be provided when the configuration file only includes configuration for the component.
* *-m \<module\>*: component module. Must be provided when the configuration file only includes configuration for the component.
* *-k*: run in scheduled mode

## yamal-stats

The **yamal-stats** utility makes use of the standard output to read channel announcements, peer announcements and data messages per channel summary from the specified YTP file.

The command syntax is

```bash
yamal-stats <path> [--data] [--channel] [--peer] [--follow]
```

Where

* *\<path\>*: (required) ytp file to read the messages
* *-d, \--data*: generates data statistics every second, displaying the number of messages read on every channel
* *-c, \--channel*: reads channel announcements
* *-p, \--peer*: reads peer announcements
* *-f, \--follow*: output appended data every second

For reading everything from a strg.ytp file:

```bash
yamal-stats strg.ytp
```

For reading only peer and channel announcements and keep waiting for new messages:

```bash
yamal-stats strg.ytp -c -p -f
```

# yamal-tail

The **yamal-tail** utility makes use of the standard output to show the raw messages written in a YTP file including information like peer, channel and associated time. The command syntax is

```bash
yamal-tail /path/to/file.ytp --follow
```

Where:

* *\<path\>*: (required) ytp file to read the messages
* *-f, \--follow*: output appended data as the file grows

# yamal-cp

The **yamal-cp** utility copies a ytp file locally. The command syntax is

```bash
yamal-cp /path/src/marketdata.ytp /path/dest/marketdata.ytp
```
