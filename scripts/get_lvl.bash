#!/bin/bash
#*****************************
# get the HOMO-LUMO levels for
# each k-points.
#  + LUMO2 + HOMO2 HOMO3 HOMO4
#*****************************
# -OVHPA XX simple bash script
# system.
#*****************************
declare -a ARR
NUMS=`grep -n "^.* .* 0.000000$" EIGENVAL | sed -e "s/^\([0-9]*\): .*/\1/g"`
prev=0
LOW_GAP=100.
LOW_LUMO=100.
LOW_LUMO2=100.
HIGH_HOMO=-100.
HIGH_HOMO2=-100.
HIGH_HOMO3=-100.
HIGH_HOMO4=-100.
FERMI=`grep fermi OUTCAR | tail -1 | awk '{printf $3}'`
echo "****************************************"
echo "FERMI LEVEL = $FERMI"
echo "****************************************"
for NUM in ${NUMS}
do
	pp=$((NUM - 1))
	if [ $pp -ne $prev ]&>/dev/null; then
		mapfile -s $((pp-4)) -n 6 ARR < EIGENVAL
		GAP=`echo ${ARR[3]} ${ARR[4]} | awk '{printf $5-$2}'`
		HOMO4=`echo ${ARR[0]} $FERMI | awk '{printf $2-$NF}'`
		HOMO3=`echo ${ARR[1]} $FERMI | awk '{printf $2-$NF}'`
		HOMO2=`echo ${ARR[2]} $FERMI | awk '{printf $2-$NF}'`
		HOMO=`echo ${ARR[3]} $FERMI | awk '{printf $2-$NF}'`
		LUMO=`echo ${ARR[4]} $FERMI | awk '{printf $2-$NF}'`
		LUMO2=`echo ${ARR[5]} $FERMI | awk '{printf $2-$NF}'`
		LOW_GAP=`echo $GAP $LOW_GAP | awk '{printf $1<$2?$1:$2 }'`
		HIGH_HOMO4=`echo $HOMO4 $HIGH_HOMO4 | awk '{printf $1<$2?$2:$1 }'`
		HIGH_HOMO3=`echo $HOMO3 $HIGH_HOMO3 | awk '{printf $1<$2?$2:$1 }'`
		HIGH_HOMO2=`echo $HOMO2 $HIGH_HOMO2 | awk '{printf $1<$2?$2:$1 }'`
		HIGH_HOMO=`echo $HOMO $HIGH_HOMO | awk '{printf $1<$2?$2:$1 }'`
		LOW_LUMO=`echo $LUMO $LOW_LUMO | awk '{printf $1<$2?$1:$2 }'`
		LOW_LUMO2=`echo $LUMO2 $LOW_LUMO2 | awk '{printf $1<$2?$1:$2 }'`
		echo $HOMO4 $HOMO3 $HOMO2 $HOMO $LUMO $LUMO2 $GAP | awk '{printf "HOMO4=%f\tHOMO3=%f\tHOMO2=%f\tHOMO=%f\tLUMO=%f\tLUMO2=%f\tGAP=%f\n",$1,$2,$3,$4,$5,$6,$7}'
	fi
	prev=$NUM
done
LOW_IGAP=`echo $HIGH_HOMO $LOW_LUMO | awk '{printf $2-$1}'`
LOW_VBGAP=`echo $LOW_LUMO $LOW_LUMO2 | awk '{printf $2-$1}'`
echo "****************************************"
echo "HIGHEST HOMO4 = $HIGH_HOMO4"
echo "HIGHEST HOMO3 = $HIGH_HOMO3"
echo "HIGHEST HOMO2 = $HIGH_HOMO2"
echo "HIGHEST HOMO = $HIGH_HOMO"
echo "LOWEST LUMO = $LOW_LUMO"
echo "LOWEST LUMO2 = $LOW_LUMO2"
echo "****************************************"
echo "LOWEST DIRECT GAP = $LOW_GAP"
echo "LOWEST INDIRECT GAP = $LOW_IGAP"
echo "LOWEST VB GAP = $LOW_VBGAP"
