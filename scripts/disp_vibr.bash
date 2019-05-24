#!/bin/bash

#--- std functions ---#
getnelec (){
# Simple atom to nelec converter
if [ -z $1 ]; then return 0; fi
case $1 in
	"H")
	return 1
	;;
	"He")
	return 2
	;;
	"Li")
	return 3
	;;
	"Be")
	return 4
	;;
	"B")
	return 5
	;;
	"C")
	return 6
	;;
	"N")
	return 7
	;;
	"O")
	return 8
	;;
	"F")
	return 9
	;;
	"Ne")
	return 10
	;;
	"Na")
	return 11
	;;
	"Mg")
	return 12
	;;
	"Al")
	return 13
	;;
	"Si")
	return 14
	;;
	"P")
	return 15
	;;
	"S")
	return 16
	;;
	"Cl")
	return 17
	;;
	"Ar")
	return 18
	;;
	"K")
	return 19
	;;
	"Ca")
	return 20
	;;
	"Sc")
	return 21
	;;
	"Ti")
	return 22
	;;
	"V")
	return 23
	;;
	"Cr")
	return 24
	;;
	"Mn")
	return 25
	;;
	"Fe")
	return 26
	;;
	"Co")
	return 27
	;;
	"Ni")
	return 28
	;;
	"Cu")
	return 29
	;;
	"Zn")
	return 30
	;;
	"Ga")
	return 31
	;;
	"Ge")
	return 32
	;;
	"As")
	return 33
	;;
	"Se")
	return 34
	;;
	"Br")
	return 35
	;;
	"Kr")
	return 36
	;;
	*)
	return 0
	;;
esac
} #end getnelec
# general cte
NATOMS_=`grep NIONS OUTCAR | sed -e "s/.* NIONS =\ *\([0-9]*\)/\1/g"`
TYP_NUM_=`grep "ions per type" OUTCAR | sed -e "s/^\ *ions per type =\ *\([0-9]* .*\)$/\1/g"`
TYP_NN_=
sx=0
for nn_ in ${TYP_NUM_}; do
	sx=`echo "$sx + $nn_"|bc -l`
	TYP_NN_=`echo "$TYP_NN_ $sx"`
done
TYP_NN_=`echo $TYP_NN_ | sed -e "s/^\ *//g"`
TYP_LAB_=`head -17 OUTCAR | grep "POTCAR:" | sed -e "s/^\ POTCAR:\ *[A-Z]*_[A-Z]*\ *\([a-zA-Z]*\).*/\1/g"`
TYP_Z=`grep ZVAL OUTCAR | tail -1 | sed -e "s/^\ *ZVAL\ *=\ *\(.*\)/\1/g"`
LINES_=`grep -n 2PiTHz OUTCAR | sed -e "s/^\([0-9]*\): .*/\1/g"`
NMODES_=`grep 2PiTHz OUTCAR | wc -l`
#prepare a xsf file
echo ANIMSTEPS $NMODES_
echo CRYSTAL
echo PRIMVEC
LL_=`grep -n -m 1 "direct lattice vectors" OUTCAR | sed -e "s/^\([0-9]*\):.*/\1/g"`
NL_=`echo $LL_ " +  3"|bc -l`
head -n $NL_ OUTCAR | tail -3 | awk '{printf "%.9f %.9f %.9f\n",$1,$2,$3}'
echo CONVVEC
head -n $NL_ OUTCAR | tail -3 | awk '{printf "%.9f %.9f %.9f\n",$1,$2,$3}'
ix=0
for LL_ in ${LINES_}; do
	ix=`echo "$ix + 1"|bc -l`
	echo PRIMCOORD $ix
	echo $NATOMS_ 1
	NL_=`echo $LL_ " +  1"|bc -l`
	spe=1
	for pi in `seq $NATOMS_`; do
		max_l=`echo $TYP_NN_ | awk '{printf $'$spe'}'`
		if [ $pi -gt "$max_l" ]; then
			spe=`echo "$spe + 1"|bc -l`
		fi
		l_spe=`echo $TYP_LAB_ | awk '{printf $'$spe'}'`
		pl_=`echo $NL_ " + " $pi|bc -l`
		po=`tail -n +$pl_ OUTCAR | head -n 1`
		getnelec $l_spe
		echo $? $po
	done

done
