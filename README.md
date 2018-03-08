# QSCAT
Qt Side Channel Analysis Tool to handle signal traces and more 

This tool aims to provide side channel analysis tool while keeping usage quite simple.
QSCAT allows you to analyze power consumption or electromagnetic data from an embedded device. You can just visualize the data.
QSCAT embeds synchronization method to align noisy traces. This is the first step to achieve key recovery.

Indeed QSCAT is a key recovery tool from signal traces. QSCAT doesn't handle signal acquisition and takes as input set of raw binaries files. A file corresponds to a signal trace or a set of traces.
Here is the steps to perform key recovery:
1. Signal traces acquistion in a binary format [out of QSCAT scope]
2. Performing Simple Power Analysis (SPA) to identify pattern to attack [QSCAT]
3. Synchronize traces if needed so that your identified pattern is align for the whole set of traces [QSCAT]
4. Launch the attack tool [QSCAT]

Thanks to QSCAT you can do:
+ Signal traces visualation
+ Signal traces synchronization
  + Manual synchronization
  + Multi threaded multi step synchronization
+ Correlation Power Analysis CPA

Here is a demo of QSCAT

<a href="http://www.youtube.com/watch?feature=player_embedded&v=oMHSy_nXtfE
" target="_blank"><img src="http://img.youtube.com/vi/oMHSy_nXtfE/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="240" height="180" border="10" /></a>
