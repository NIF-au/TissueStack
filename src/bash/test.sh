#!/bin/bash
for x in {0..6}
	do
	for z in x y z
	do
		for i in {0..2000}
			do
  				if [ -f "$x/$z/$i.low.res.png" ]
				then
					mv $x/$z/$i.low.res.png $x/$z/$i/
					rm $x/$z/$i.png
   				else
					echo "break"
					break
   			fi
		done
	done
done
