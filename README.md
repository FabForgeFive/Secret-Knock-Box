# Secret-Knock-Box
Secret Knock Box project, with Arduino code etc.

For Important Stuff

The SECRET KNOCK BOX is a keepsake box that only opens when its owner taps their custom secret knock.  The knock can be reprogrammed at any
time.  The box is powered by 3 AAA batteries, and uses an Arduino Pro Mini, a piezo pickup (knock detector), a sub micro servo (latch
release), and custom 3D printed and laser-cut parts.  

To open the box

Demonstrated in the video on the fabforgefive website. Slide the power switch to the right. Listen for two buzzes. Now tap your secret
knock on or near the circle on the bottom. If your knock is wrong, the book will buzz.  If correct, the book will spring open. 

To set a new secret knock

Open the book with the old knock. With the power on, push and hold the inside button. Tap your new knock. The box LED will blink the knock
back to you, then save your new knock to memory for the next time you use the box. The Arduino will save your new knock into its EEPROM
memory, where it will stay unless you change it again.

The Arduino code (sketch) contains code from Secret-Knock Gumball Machine written by Steve Hoefer, used with permission. Steve’s Gumball
Machine was the first Arduino project that my kids and I built, and was the gateway to many exciting family projects over the years. Thanks 
Steve for the inspiration!

Build tutorial is on the fabforgefive website.
