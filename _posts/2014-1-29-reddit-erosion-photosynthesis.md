---
title: Reddit and erosion
layout: default
---

So, what do these two things have in common?

Last Saturday I debuted the simulation to the fine folk of the [/r/worldbuilding subreddit](www.reddit.com/tb/1w4dy5). Its a small community dedicated to the creation of sci fi and fantasy worlds with a strong emphasis on mapmaking. I've posted there a [few](http://www.reddit.com/tb/oOyw3) [times](http://www.reddit.com/r/worldbuilding/comments/1uuhxe/a_scientific_approach_to_dark_elves/), generally demonstrating a toy world I modeled by hand. This was actually the project that lead up to the creation of [PyTectonics](http://pytectonics.sourceforge.net/), the predecessor to the simulation you see here.

The feedback there was very informative. I was surprised to see interest in some things I'd previously thought the public wouldn't pay much attention to in a model - sea floor height, for instance. Other suggestions, on the other hand, I knew would be brought up at some point - the comments made very clear the need for realistic mountains. While the model at the time still technically simulated mountains, the lack of an erosion model was presumably causing them to get out of hand, eventually occupying every available cell of continental crust. An erosion model would be needed before mountains could be integrated into the main satellite view.

Fortunately, I've had a [solid erosion model](https://github.com/davidson16807/tectonics.js/raw/master/research/simoes2010.pdf) lined up for some time, now. What's beautiful about this model is its ability to simulate erosion over large space and time scales, such as those you'd expect from a plate tectonics simulator. I'd seen many published erosion models while researching the problem, but almost all of them dealt with the spatial scales and timespans you'd need to, say, model the formation of a river system. 

Don't let the dense prose fool you - the model is stupid simple at its core. For each pair of neighboring grid cells, the model takes the height difference between the two and transfers a fraction of that height from one cell to another. The fraction that gets transferred is determined by two parameters: 

1.) precipitation

2.) rate of erosion, expressed as a fraction of height per unit of precipitation

That's it. The original publication embellishes upon this to partition bedrock erosion from sediment erosion, and I might do something like this in the future, but for now the model works good enough. Precipitation is also currently set to a constant reflecting the average rainfall experienced on Earth, but again, good enough.

So mountains are less frequent, now. There are still issues though:

1.) Mountains are still much taller than those encountered on earth, say, 15km.

2.) Coasts occassionally appear much more rounded than what we see on earth.

Both may be symptoms for a number of problems that are both hard to pin down. It may be that the erosion parameters require calibration, but any parameter changes made to address one issue will likely exacerbate the other. Setting precipitation to something besides a constant may be a more plausible solution. Coasts may not be as well rounded if they reflect gradients in precipitation, doubly so if certain regions receive hardly any precipitation, whatsoever. Mountain height might still be overpredicted, but at least then calibration could be conducted on it without exacerbating issues with the coastline. Issues with mountain height may also be addressed by calibrating parameters that define the rate at which continental crust accretes. Previously I was wary to touch these parameters since the model was already predicting a reasonable landmass percentage, but landmass appears to have increased in size now that there is erosion, and it might be time to reinvestigate the accretion parameters.