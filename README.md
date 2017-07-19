# QSCAT
Qt Side Channel Analysis Tool to handle signal traces and more 

This tool aims to provide side channel analysis tool while keeping usage quite simple.
QSCAT allows you to analyze power consumption or electromagnetic data from an embedded device. You can just visualize the data.
QSCAT embeds synchronization method to align noisy traces. This is the first step to achieve key recovery.

Indeed QSCAT is a key recovery tool from signal traces. QSCAT doesn't handle signal acquisition and takes as input set of raw binaries files. A file corresponds to a signal traces.
Here is the steps to perform key recovery:
1. Signal traces acquistion [out of QSCAT scope]
2. Performing Simple Power Analysis (SPA) to identify pattern to attack [QSCAT]
3. Synchronize traces if needed so that your identified pattern is align for the whole set of traces [QSCAT]
4. Launch the attack tool [not yet developped]

Thanks to QSCAT you can do:
+ Signal traces visualation [done => 100%]
+ Signal traces synchronization [functionnal => 70%]
  + Manual synchronization
  + Multi threaded automatic synchronization [Sum of Differences (SOD) lacks some other methods]
+ Correlation Power Analysis CPA [0%]
+ Differential Power Analysis DPA [0%]
+ Template Analysis [0%]



Full screen visualization: ![picture](https://i.imgur.com/pnDbuRP.png "Full screen visualization")
Before synchronization: ![picture](https://i.imgur.com/DLZCLIq.png "Before synchronization")
After synchronization: ![picture](https://i.imgur.com/hfLA8Pz.png "After synchronization")
