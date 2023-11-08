# SENT-SAEJ2716
A SENT SAE J2716 "cry baby" using a Nano to send SENT data. Useful notes within.
On the job we have a new product that might be using SAE J2716. So for test purposes I created this Arduino NANO program that just spews SENT data. Also within, the CRC calculation algorithm, and some notes and findings. 
What I needed to do was generate the data - since the actual device is not ready yet - and use that to test decoding scopes to see which ones were good at it. SENT is not a common protocol to decode with scopes, but a USB PICO 2205 did the job. 
Another important trick was the use of PORT writing in the Arduino for speed. Digital Writes with high and low were too slow. Anything to cause lag was a problem. But I managed to get the Nano operating at a 15uS tick time, which is OK for the most part. 
This code now only serves as an example. I had no reason to write a decoder .... yet. 
