#!/bin/bash

for i in icons/*.xpm;do
	translate $i ${i/xpm/bits} bits
done

if [ ! -e icons/fileopen.bits ]; then
	echo "Error generating icons.  Ensure XPMTranslator is installed."
	exit 1
fi

xres -o BGhostview_icons.rsrc \
	-a bits:660:fileopen icons/fileopen.bits \
	-a bits:661:firstpage icons/firstpage.bits \
	-a bits:662:lastpage icons/lastpage.bits \
	-a bits:663:next icons/next.bits \
	-a bits:664:nextpage icons/nextpage.bits \
	-a bits:665:prevpage icons/prevpage.bits \
	-a bits:666:printpage icons/printpage.bits \
	-a bits:667:reload icons/reload.bits \
	-a bits:668:zoomin icons/zoomin.bits \
	-a bits:669:zoomout icons/zoomout.bits

rm -f icons/*.bits

rc -d BGhostview_icons.rsrc -o BGhostview.rdef
rm BGhostview_icons.rsrc
cat BGhostview_basic.rdef >> BGhostview.rdef
