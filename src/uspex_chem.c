#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"

/* Convert an USPEX Individuals file to a chemical
 * potential gnuplot script, using a reference for
 * each component (given separately).
 *
 * Optional file chem.in: for reference potentials
 * can be written as in the example:
 * -----------------------------------------------
 * 1 2 -16.625834
 * 2 1 -0.21177758
 * -----------------------------------------------
 * first line: potential for the 1st species which
 * consists of 2 atoms as defined in atomType, and
 * have an energy of -16.625834 eV.
 * second line: the 2nd reference has 1 atom, with
 * an energy of -0.21177758 eV.
 * If the file chem.in is missing or one reference
 * is omitted, it is replaced by the USPEX results
 * for the minimum energy of that species.
 *
 * Obviously, this does only make sense in a USPEX
 * >>>>>> VARIABLE COMPOSITION calculation! <<<<<<
 *
 * secret keyword 'pictave' removes the last atom.
 *
 * ------------------- (c) OVHPA: Okadome Valencia
 * mail: hubert.valencia _at_ imass.nagoya-u.ac.jp */

/*structure*/
typedef struct {
	UINT natoms;
	UINT *atoms;
	DOUBLE energy;
	DOUBLE e_form;
} individual;
typedef struct {
	individual *refs;
	individual *inds;
	DOUBLE  e_f_min;
	DOUBLE  e_f_max;
} handle;
/*globals*/
UINT n_individuals;
UINT n_species;
handle  ih;
BOOL pictave;

/*specific*/
#define NIND (n_individuals)
#define NREF (n_species)
#define IND ih.inds
#define REF ih.refs

BOOL read_ind_param(){
	FILE *vf;
	size_t len = 0;
	CHAR *line = NULL;
	CHAR *ptr , *ptr2;
	vf = fopen("Individuals", "rt");
	if (!vf) return -1;/*FAIL*/
	GET_LAST_LINE(vf,line);
	ptr=&(line[0]);
	SKIP_BLANK(ptr);
	SKIP_NUM(ptr);
	SKIP_BLANK(ptr);
	GET_UINT(NIND,ptr,ptr2);
	if((ptr2==NULL)||(NIND<1)) {
		fclose(vf);
		return FALSE;
	}
	NIND++;/*index 0 to NIND -> NIND+1 values*/
	ptr=ptr2+1;
	while((*ptr!='[')&&(*ptr!='\0')) ptr++;
	if(*ptr=='\0') return FALSE;
	ptr=ptr+1;NREF=0;
	SKIP_BLANK(ptr);
	while((*ptr!=']')&&(*ptr!='\0')){
		SKIP_NUM(ptr);
		ptr=ptr+1;
		NREF++;
		SKIP_BLANK(ptr);
	}
	fclose(vf);
	if(NREF<1) return FALSE;
	ALLOC(REF,NREF,individual);
	ALLOC(IND,NIND,individual);
	ih.e_f_min = -1.0;
	ih.e_f_max =  0.0;
	FREE(line);
	return TRUE;
}

BOOL read_individuals(){
	FILE *vf;
	size_t len = 0;
	CHAR *line = NULL;
	CHAR *ptr , *ptr2;
	UINT num, idx, jdx;
	BOOL is_reduce;
	/**/
	vf = fopen("Individuals", "rt");
	if (!vf) return FALSE;/*FAIL*/
	/*FILL IND*/
	READLINE(vf,line);
	/*at present supercell calculation is not supported*/
	if(STRFIND("SuperCell",line) !=NULL) return FALSE;/*FAIL*/
	READLINE(vf,line);
	if(STRFIND("eV/atom",line) != NULL) {
		is_reduce=TRUE;
		READLINE(vf,line);
	} else is_reduce=FALSE;
	ptr=&(line[0]);
	SKIP_BLANK(ptr);
	while(!ISDIGIT(*ptr)){
		/*skip additionnal header lines, if any*/
		READLINE(vf,line);
		ptr=&(line[0]);
		SKIP_BLANK(ptr);
	}
	/*READ*/
	num=0;idx=0;
/*NEW: create the fake 0 structure*/
	ALLOC(IND[0].atoms,NREF,UINT);
	IND[0].natoms=0;
	for(jdx=0;jdx<NREF;jdx++) IND[0].atoms[jdx]=0;
	while((idx<NIND)&&(!feof(vf))){
		ptr=&(line[0]);
		SKIP_BLANK(ptr);
		SKIP_NUM(ptr);/*skip generation*/
		SKIP_BLANK(ptr);
		GET_UINT(idx,ptr,ptr2);
#ifdef _DEB_
fprintf(stdout,"read: idx=%i ",idx);
#endif
		if(ptr2==NULL) goto end_loop;/*no structure number -> can't read line*/
		ptr=ptr2+1;
		while((*ptr!='[')&&(*ptr!='\0')) ptr++;
		if(*ptr=='\0') goto end_loop;/*no composition -> can't read line*/
		IND[idx].natoms=0;ptr2=ptr;jdx=0;
/*we need to know nspecies before*/
		ALLOC(IND[idx].atoms,NREF,UINT);
#ifdef _DEB_
fprintf(stdout,"[ ");
#endif
		ptr=ptr2+1;
		SKIP_BLANK(ptr);
		while((ptr2!=NULL)&&(*ptr2!='\0')&&(jdx<NREF)){
			GET_UINT(IND[idx].atoms[jdx],ptr,ptr2);
#ifdef _DEB_
fprintf(stdout,"%i ",IND[idx].atoms[jdx]);
#endif
			IND[idx].natoms+=IND[idx].atoms[jdx];
			jdx++;
			ptr=ptr2+1;
			SKIP_BLANK(ptr);
		}
		if(pictave){/*skip the last atom*/
			while((*ptr!=']')&&(*ptr!='\0')) ptr++;
		}
#ifdef _DEB_
fprintf(stdout,"] ");
#endif
		if(*ptr=='\0') goto end_loop;/*composition does not end -> can't read line*/
#ifdef _DEB_
fprintf(stdout,"atoms=%i ",IND[idx].natoms);
#endif
		if(*ptr==']') ptr++;
		SKIP_BLANK(ptr);
		/*get energy*/
		GET_DOUBLE(IND[idx].energy,ptr,ptr2);
		if(is_reduce) IND[idx].energy*=IND[idx].natoms;/*un-reduced energy*/
#ifdef _DEB_
fprintf(stdout," energy=%lf",IND[idx].energy);
#endif
		/*SKIP rest of the line*/
end_loop:
		num++;
#ifdef _DEB_
fprintf(stdout," [line=%i]\n",num);
#endif
		READLINE(vf,line);
	}
	FREE(line);
	fclose(vf);
	return TRUE;
}

BOOL read_chem(){
	FILE *vf;
	size_t len = 0;
	DOUBLE energy_ref;
	UINT species,nspe;
	CHAR *line=NULL;
	CHAR *ptr,*ptr2;
/* READ chem.in file */
	pictave=FALSE;
        vf=fopen("chem.in", "rt");
        if(!vf) return FALSE;/*FAIL*/
	READLINE(vf,line);
	while(!feof(vf)){
		species=0;nspe=0;energy_ref=0.;
		ptr=&(line[0]);
		if((STRFIND("pictave",line) != NULL)
			||(STRFIND("Pictave",line) != NULL)
			||(STRFIND("PICTAVE",line) != NULL))
		{
			pictave=TRUE;
			READLINE(vf,line);
			continue;
		}
		GET_UINT(species,ptr,ptr2);
		if(ptr2==NULL){/*skip?*/
			READLINE(vf,line);
			continue;
		}
		ptr=ptr2+1;
		GET_UINT(nspe,ptr,ptr2);
		if(ptr2==NULL){/*skip?*/
			READLINE(vf,line);
			continue;
		}
		ptr=ptr2+1;
		GET_DOUBLE(energy_ref,ptr,ptr2);
		if((species<1)||(nspe<1)||(energy_ref==0.)){
			/*wrong values: SKIP*/
			READLINE(vf,line);
			continue;
		}
		species--;
		ALLOC(REF[species].atoms,NREF,UINT);
		REF[species].atoms[species]=nspe;
		REF[species].natoms=nspe;
		REF[species].energy=energy_ref;
		READLINE(vf,line);
	}
	if(pictave) n_species--;
	FREE(line);
	fclose(vf);
	return TRUE;
}

BOOL create_chem(){
	BOOL is_ref;
	UINT idx,jdx,kdx;
	DOUBLE e_ref=0.0;
	for(jdx=0;jdx<NREF;jdx++){
		ALLOC(REF[jdx].atoms,NREF,UINT);
		e_ref=1.0/0.0;/*+inf <- SHOULD USE DBL_MAX ???*/
		for(idx=1;idx<NIND;idx++){
			/*jdx!=0 and other == 0*/
			is_ref=TRUE;
			for(kdx=0;kdx<NREF;kdx++){
				if((jdx!=kdx)&&(IND[idx].atoms[kdx]!=0)) is_ref=FALSE;
				if((jdx==kdx)&&(IND[idx].atoms[kdx]==0)) is_ref=FALSE;
			}
			if(is_ref){
				/*we have a ref*/
				if((IND[idx].energy/IND[idx].natoms) <= e_ref){
					e_ref=IND[idx].energy/(DOUBLE)IND[idx].natoms;
#ifdef _DEB_
fprintf(stdout,"IND[%i] accepted for REF[%i] e_ref=%lf\n",idx,jdx,e_ref);
#endif
					REF[jdx].energy=IND[idx].energy;
					REF[jdx].natoms=IND[idx].natoms;
					if(REF[jdx].atoms==NULL) ALLOC(REF[jdx].atoms,NREF,UINT);
					for(kdx=0;kdx<NREF;kdx++) REF[jdx].atoms[kdx]=IND[idx].atoms[kdx];
				}
			}
		}
		if(e_ref>0) return FALSE;
	}
	return TRUE;
}

void calc_form(){
	DOUBLE ratio,e_f;
	UINT idx,jdx,acc;
	individual  tamp;/*for _SWAP_*/
/*ASSERT: REF set must be complete!*/
	for(idx=1;idx<NIND;idx++){
		ratio=0.;e_f=0.;
		if(IND[idx].energy>10000.000) {
			IND[idx].e_form=e_f+IND[idx].energy;
			IND[idx].e_form=10000.000;
			continue;
			/*it won't be displayed anyway due to ih.e_f_max*/
		}
		for(jdx=0;jdx<NREF;jdx++){
			//FIXME: calculate _acc_
			ratio=IND[idx].atoms[jdx]/(DOUBLE)REF[jdx].natoms;
			if(ratio==0.) continue;
			e_f-=ratio*REF[jdx].energy;
		}
		IND[idx].e_form=e_f+IND[idx].energy;
		if(ih.e_f_min > IND[idx].e_form) ih.e_f_min=IND[idx].e_form;
		if(ih.e_f_max < IND[idx].e_form) ih.e_f_max=IND[idx].e_form;
	}
	/*order everything based on e_form <- necessary?*/
#ifdef _SWAP_
	for(idx=1;idx<NIND;idx++){
		acc=idx;
		for(jdx=acc+1;jdx<NIND;jdx++){
			if(IND[jdx].e_form<IND[acc].e_form) acc=jdx;
		}
		if(acc!=idx){
			//swap idx <-> acc
			tamp.natoms = IND[idx].natoms;
			tamp.atoms  = IND[idx].atoms;
			tamp.energy = IND[idx].energy;
			tamp.e_form = IND[idx].e_form;
			IND[idx].natoms = IND[acc].natoms;
			IND[idx].atoms  = IND[acc].atoms;
			IND[idx].energy = IND[acc].energy;
			IND[idx].e_form = IND[acc].e_form;
			IND[acc].natoms = tamp.natoms;
			IND[acc].atoms  = tamp.atoms;
			IND[acc].energy = tamp.energy;
			IND[acc].e_form = tamp.e_form;
		}
	}
#endif
}

BOOL prep_hull(UINT species){
        FILE *vf;
	DOUBLE p,e_form;
	DOUBLE  dx,de,x;
	CHAR *line=NULL;
	UINT  idx,jdx,mid,min,max;
	individual tamp;/*_SWAP_*/
	/**/
	ALLOC(line,64,CHAR);
	sprintf(line,"chem_hull_%i.gnuplot",species+1);
	vf=fopen(line,"w");
	if(!vf) return -1;
	/*WRITE: header*/
	fprintf(vf,"#!/usr/bin/gnuplot\n");
#ifdef _DEB_
/*not so sure these options are useful*/
	fprintf(vf,"set terminal qt dashed\n");
	fprintf(vf,"set termoption dash\n");
#endif
	fprintf(vf,"unset colorbox\n");
	fprintf(vf,"set for [i=1:5] linetype i dt i\n");
	fprintf(vf,"set style line 1 lt -1 lw 1 pt 1 linecolor rgb \"black\"\n");
	fprintf(vf,"set style line 2 lt 1 lw 2 pt 1 linecolor rgb \"red\"\n");
	fprintf(vf,"set style line 3 lt 1 lw 2 pt 1 linecolor rgb \"green\"\n");
	fprintf(vf,"set style line 4 lt 1 lw 2 pt 1 linecolor rgb \"blue\"\n");
	fprintf(vf,"set style line 5 lt 2 lw 2 pt 1 linecolor rgb \"red\"\n");
	fprintf(vf,"set style line 6 lt 2 lw 2 pt 1 linecolor rgb \"green\"\n");
	fprintf(vf,"set style line 7 lt 2 lw 2 pt 1 linecolor rgb \"blue\"\n");
	fprintf(vf,"xmin=0\n");
	fprintf(vf,"xmax=100\n");
	fprintf(vf,"ymin=%lf\n",ih.e_f_min);
	fprintf(vf,"ymax=%lf\n",ih.e_f_max);
	fprintf(vf,"set yrange [ymin:ymax]\n");
	fprintf(vf,"set xrange [xmin:xmax]\n");
	fprintf(vf,"set samples 10000\n");
/*order everything based on %x*/
	for(idx=1;idx<NIND;idx++){
		min=idx;
		p=IND[idx].atoms[species]/(DOUBLE)(IND[idx].natoms);
		for(jdx=min+1;jdx<NIND;jdx++){
			e_form=IND[jdx].atoms[species]/(DOUBLE)IND[jdx].natoms;
			if(e_form <= p) {
				p=e_form;
				min=jdx;
				}
		}
		if(min!=idx){
			/*swap idx <-> min*/
			tamp.natoms = IND[idx].natoms;
			tamp.atoms  = IND[idx].atoms;
			tamp.energy = IND[idx].energy;
			tamp.e_form = IND[idx].e_form;
			IND[idx].natoms = IND[min].natoms;
			IND[idx].atoms  = IND[min].atoms;
			IND[idx].energy = IND[min].energy;
			IND[idx].e_form = IND[min].e_form;
			IND[min].natoms = tamp.natoms;
			IND[min].atoms  = tamp.atoms;
			IND[min].energy = tamp.energy;
			IND[min].e_form = tamp.e_form;
		}
	}
/*end: order*/
	/*WRITE: convex hull*/
/*1: find the lowest formation energy*/
	min=1;
	for(idx=1;idx<NIND;idx++) if(IND[idx].e_form <= IND[min].e_form) min=idx;
	fprintf(vf,"x_%i=%lf\n",min,100*IND[min].atoms[species]/(DOUBLE)IND[min].natoms);
	fprintf(vf,"e_%i=%lf\n",min,IND[min].e_form);
/*2: left*/
	mid=min;max=min;
	while(min>1){
		/*select the first mid with %[mid] != %[min]*/
		x=IND[min].atoms[species]/(DOUBLE)IND[min].natoms;
		dx=x;
		while((dx==x)&&(mid>1)){
			mid--;
			dx=IND[mid].atoms[species]/(DOUBLE)IND[mid].natoms;
		}
		if(mid==1) break;
		dx=x-IND[mid].atoms[species]/(DOUBLE)IND[mid].natoms;
		p=fabs((IND[mid].e_form-IND[min].e_form)/dx);
		for(idx=mid-1;idx>0;idx--){
			dx=x-(IND[idx].atoms[species]/(DOUBLE)IND[idx].natoms);
			de=fabs((IND[idx].e_form-IND[min].e_form)/dx);
			if(de<p){
				p=de;
				mid=idx;
			}
		}
		fprintf(vf,"x_%i=%lf\n",mid,100*IND[mid].atoms[species]/(DOUBLE)IND[mid].natoms);
		fprintf(vf,"e_%i=%lf\n",mid,IND[mid].e_form);
		fprintf(vf,"set arrow from x_%i,e_%i to x_%i,e_%i ls 2 nohead\n",min,min,mid,mid);
		min=mid;
	}
/*3: right*/
	min=max;mid=max;
	while(max<NIND){
		/*select the first mid with %[mid] != %[min]*/
		x=IND[min].atoms[species]/(DOUBLE)IND[min].natoms;
		dx=x;
	mid++;
	while((dx==x)&&(mid<NIND)){
		dx=IND[mid].atoms[species]/(DOUBLE)IND[mid].natoms;
		mid++;
	}
	if(mid==NIND) break;
	dx=x-IND[mid].atoms[species]/(DOUBLE)IND[mid].natoms;
	p=fabs((IND[mid].e_form-IND[min].e_form)/dx);
	for(idx=mid+1;idx<NIND;idx++){
		dx=x-(IND[idx].atoms[species]/(DOUBLE)IND[idx].natoms);
		de=fabs((IND[idx].e_form-IND[min].e_form)/dx);
		if(de<p){
			p=de;
			mid=idx;
		}
	}
	fprintf(vf,"x_%i=%lf\n",mid,100*IND[mid].atoms[species]/(DOUBLE)IND[mid].natoms);
	fprintf(vf,"e_%i=%lf\n",mid,IND[mid].e_form);
	fprintf(vf,"set arrow from x_%i,e_%i to x_%i,e_%i ls 2 nohead\n",min,min,mid,mid);
	min=mid;
	}
	/*WRITE: data*/
	fprintf(vf,"plot \"-\" w points ls 1 notitle\n");
	for(idx=1;idx<NIND;idx++){
		p=IND[idx].atoms[species]/(DOUBLE)IND[idx].natoms;
		fprintf(vf,"%lf %lf\n",p*100.,IND[idx].e_form);
	}
	fprintf(vf,"e\n");
	fprintf(vf,"\n");
	/*WRITE: eps output*/
	fprintf(vf,"print \"ready for printing?\"\n");
	fprintf(vf,"pause -1\n");
	fprintf(vf,"set term postscript enhance font \"DejaVuSans-Bold,12\" lw 2\n");
	fprintf(vf,"set output \"chem_hull_%i.eps\"\n",species+1);
	fprintf(vf,"set style line 1 lt -1 lw 2 pt 1 linecolor rgb \"black\"\n");
	fprintf(vf,"set key bottom center outside horizontal\n");
	fprintf(vf,"set title \"VARIABLE COMPOSITION species %i\" font \"DejaVuSans-Bold,24\"\n",species+1);
	fprintf(vf,"set xlabel \"atomic ratio [%i] (%%)\" font \"DejaVuSans-Bold,16\"\n",species+1);
	fprintf(vf,"set ylabel \"Formation energy (eV)\" font \"DejaVuSans-Bold,16\"\n");
	fprintf(vf,"set yrange [ymin:ymax]\n");
	fprintf(vf,"set xrange [xmin:xmax]\n");
	fprintf(vf,"set samples 10000\n");
	fprintf(vf,"replot\n");
	fprintf(vf,"pause 1\n");
	/*EOF*/
	FREE(line);
	fclose(vf);
	return TRUE;
}

void free_all(){
	UINT idx;
	for(idx=0;idx<NIND;idx++){
		FREE(IND[idx].atoms);
		IND[idx].atoms=NULL;
	}
	FREE(IND);
	IND=NULL;
	for(idx=0;idx<NREF;idx++){
		FREE(REF[idx].atoms);
		REF[idx].atoms=NULL;
	}
	FREE(REF);
	REF=NULL;
	NIND=0;
	NREF=0;
}

int main(void){
	UINT idx,jdx;
	BOOL chem;
	if(!read_ind_param()){
		fprintf(stderr,"ERROR reading Individuals!\n");
		return -1;
	}
	chem=read_chem();
	if(!read_individuals()){
		fprintf(stderr,"ERROR reading Individuals!\n");
		return -1;
	}
	if(chem==FALSE) {
//		pictave=TRUE;NREF--;
		if(!create_chem()){
			fprintf(stderr,"ERROR creating chem.in!\n");
			return -1;
		}
	}
	calc_form();
	for(idx=1;idx<NIND;idx++){
		fprintf(stdout,"IND[%i] E=%lf\n",idx,IND[idx].energy);
	}
	for(idx=0;idx<NREF;idx++){
		prep_hull(idx);
		fprintf(stdout,"REF[%i] [ ",idx);
		for(jdx=0;jdx<NREF;jdx++) fprintf(stdout,"%i ",REF[idx].atoms[jdx]);
		fprintf(stdout,"] E=%lf\n",REF[idx].energy);
	}
	free_all();
	return 0;
}




