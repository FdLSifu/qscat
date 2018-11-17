# How to recover the key

1. Launch qscat
2. Click top left button to open traces binary file named `2016.06.01-12.00.09_traces_x1000x9996x4.bin`
3. Click `Open DataSet` to open text binary file named `2016.06.01-12.00.09_textin_x1000x9996x4.bin` 
3. Click `Synchronization` to synchronize the traces
4. Click `Add Step` to add a synchronization step 
5. Here is the step 1 synchro configuration
* Synchro Method: Sum Of Differences
* Reference Curve: 2016.06.01-12.00.09_textin_x1000x9996x4.bin:0
* Reference Pattern: 1100-1200
* Window Search: -300:300
* Precision: 1
6. Click on `Run Synchro` the Synchronization Threshold should not present any significant peaks (except at O)
7. Close Syncrho window and click `Attack` button
8. Here is the attack configuration
* Attack method: CPA
* Algorithm: AES
* Function: AES output SBOX
* Points Range: 1000-1500
* Number of traces: 1000
* Bytes Index: All (0-15 selected)
9. Click `Launch Attack`
