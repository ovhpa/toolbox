#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

void dump_help(void){
#define _CR fprintf(stdout,"\n");
  fprintf(stdout,"--------------------------------------------------");_CR;
  fprintf(stdout,"......pdos command help (version 0.1 - 2020)......");_CR;
  fprintf(stdout,"--------------------------------------------------");_CR;
  fprintf(stdout,"  -a X (select atom X for sum)     <- one per atom");_CR;
  fprintf(stdout,"  -amin X (atom list from  X) select atoms from  X");_CR;
  fprintf(stdout,"if no -amax used, selection ends at the last  atom");_CR;
  fprintf(stdout,"  -amax Y (atom list up to Y) select atoms up to Y");_CR;
  fprintf(stdout,"if no -amin used, selection starts from first atom");_CR;
  fprintf(stdout,"  -n X (select dos X for sum)       <- one per dos");_CR;
  fprintf(stdout,"  -nmin X (dos list from  X)    select dos from  X");_CR;
  fprintf(stdout,"if no -nmax used, selection ends at the last DOS  ");_CR;
  fprintf(stdout,"  -nmax Y (dos list up to Y)    select dos up to Y");_CR;
  fprintf(stdout,"if no -nmin used, selection starts from first DOS ");_CR;
  fprintf(stdout,"  -u select spin up dos only <- valid if spin used");_CR;
  fprintf(stdout,"  -d select spin down dos only    <- same as above");_CR;
  fprintf(stdout,"if both -u and -d are used, the sum is performed..");_CR;
  fprintf(stdout,"which is the SAME as no -u and -d and spin present");_CR;
  fprintf(stdout,"  -l label <- label is the name of the output file");_CR;
  fprintf(stdout,"if no name provided, output is stdout ie. terminal");_CR;
  fprintf(stdout,"  -h do nothing and displaying this help page.....");_CR;
  fprintf(stdout,"--------------------------------------------------");_CR;
  fprintf(stdout,".....INDEX of projected DOS (for -n switches).....");_CR;
  fprintf(stdout,"...0...|....1...|.2|.3|.4|.5.|.6.|.7.|.8.|....9...");_CR;
  fprintf(stdout,"all DOS|s(x2+y2)|py|pz|px|dxy|dyz|dz2|dxz|d(x2-y2)");_CR;
  fprintf(stdout,"....10....|.11.|.12.|.13|.14.|....15...|....16....");_CR;
  fprintf(stdout,"fy(3x2-y2)|fxyz|fyz2|fz3|fxz2|fz(x2-y2)|fx(x2-3y2)");_CR;
  fprintf(stdout,"--------------------------------------------------");_CR;
#undef _CR
}

void dump_dos(FILE *target,double *energy,double *dos_val,int total_dos){
  int index;
  double sum=0.;
  for(index=1;index<total_dos;index++) sum+=dos_val[index];
  fprintf(target,"#integration: %G\n",sum);
  for(index=1;index<total_dos;index++) fprintf(target,"%G %G\n",energy[index],dos_val[index]);
}

int main (int argc, char *argv[]){
  FILE *fp = NULL;
  FILE *wp = NULL;
  PREP_READLINE();
  char *line=NULL;
  char *ptr,*ptr2;
  int   index = 0;
  int   n_dos = 0;
  int   n_atoms=0;
  int  temp_num=0;
  int spin_up = TRUE;
  int spin_dn = TRUE;
  int  total_atoms=0;
  int  total_dos = 0;
  int is_partial = 0;
  int  have_spin = 0;
  double e_fermi=0.0;
  char *label = NULL;
  int  *idx_dos=NULL;
  int *idx_atom=NULL;
  double ignore_d=0.;
  double  temp_value;
  double    temp_sum;
  int  verbose=FALSE;
/*intervals*/
  int from_a=0;
  int to_a = 0;
  int  *tmp_idx=NULL;
  int from_n=0;
  int to_n = 0;
  int  max_a=0;
  int  max_n=0;
  int   _idx=0;
  int   _jdx=0;
/*final arrays*/
  double  *energy=NULL;
  double *dos_val=NULL;
/*defines*/
#define IDX_CP(src,dest,num) do{\
	int __idx=0;\
	while(__idx < (num)) {\
		(dest)[__idx]=(src)[__idx];\
		__idx++;\
	}\
}while(0)
//BEGIN
  if(argc<2) {
    //all default
  }else{
    //get switch values
    index=1;
    while(index<argc){
      if(argv[index][0]=='-'){
        //we have a switch
	switch (argv[index][1]){
	case 'a':
	  ptr=&(argv[index][2]);
	  if(*ptr=='m'){
            //min or max
            if(*(ptr+1)=='i'){
              //ptr+2 is assumed to be n
              //try to get ptr+3
              ptr+=3;SKIP_BLANK(ptr);
	      GET_UINT(from_a,ptr,ptr2);
	      if(ptr==ptr2){
		//second chance if -amin X
		// then X is on next index
		if(ISDIGIT(argv[index+1][0])){
		  //BINGO
		  ptr=&(argv[index+1][0]);
		  GET_UINT(from_a,ptr,ptr2);
		  if(ptr==ptr2){//this should not happen
		    dump_help();
		    return 0;
		  }
		  index++;//to account for the value
		}else{
		  dump_help();
		  return 0;
		}
	      }
	    }else if(*(ptr+1)=='a'){
	      //ptr+2 is assumed to be x
	      //try to get ptr+3
	      ptr+=3;SKIP_BLANK(ptr);
	      GET_UINT(to_a,ptr,ptr2);
	      if(ptr==ptr2){
		//second chance if -amax X
		// then X is on next index
		if(ISDIGIT(argv[index+1][0])){
		  //BINGO
		  ptr=&(argv[index+1][0]);
		  GET_UINT(to_a,ptr,ptr2);
		  if(ptr==ptr2){//this should not happen
		    dump_help();
		    return 0;
		  }
		  index++;//to account for the value
		}else{
		  dump_help();
		  return 0;
	        }
	      }
	    }else{
	      //unknown key
	      dump_help();
	      return 0;
	    }
	  }else{
	    SKIP_BLANK(ptr);
	    GET_UINT(temp_num,ptr,ptr2);
	    if((ptr==ptr2)||(temp_num<1)){
	      //second chance if -a X
	      // then X is on next index
	      if(ISDIGIT(argv[index+1][0])){
		//BINGO
		ptr=&(argv[index+1][0]);
		GET_UINT(temp_num,ptr,ptr2);
		if(ptr==ptr2){//this should not happen
		  dump_help();
		  return 0;
		}
		index++;//to account for the value
	      }else{
		dump_help();
		return 0;
	      }
	    }
	    //register atom for sum
	    n_atoms++;
	    ALLOC(tmp_idx,n_atoms,int);
	    IDX_CP(idx_atom,tmp_idx,n_atoms-1);
	    tmp_idx[n_atoms-1]=temp_num;
	    FREE(idx_atom);
	    idx_atom=tmp_idx;
	    if(temp_num>max_a) max_a=temp_num;
	  }
	  break;
	case 'n':
          ptr=&(argv[index][2]);
          if(*ptr=='m'){
            //min or max
            if(*(ptr+1)=='i'){
              //ptr+2 is assumed to be n
              //try to get ptr+3
              ptr+=3;SKIP_BLANK(ptr);
              GET_UINT(from_n,ptr,ptr2);
              if(ptr==ptr2){
		//second chance if -nmin X
		// then X is on next index
		if(ISDIGIT(argv[index+1][0])){
		  //BINGO
		  ptr=&(argv[index+1][0]);
		  GET_UINT(from_n,ptr,ptr2);
		  if(ptr==ptr2){//this should not happen
		    dump_help();
		    return 0;
		  }
		  index++;//to account for the value
		}else{
                  dump_help();
                  return 0;
                }
	      }
            }else if(*(ptr+1)=='a'){
              //ptr+2 is assumed to be x
              //try to get ptr+3
              ptr+=3;SKIP_BLANK(ptr);
              GET_UINT(to_n,ptr,ptr2);
              if(ptr==ptr2){
		//second chance if -nmax X
		// then X is on next index
		if(ISDIGIT(argv[index+1][0])){
		  //BINGO
		  ptr=&(argv[index+1][0]);
		  GET_UINT(to_n,ptr,ptr2);
		  if(ptr==ptr2){//this should not happen
		    dump_help();
		    return 0;
		  }
		  index++;//to account for the value
		}else{
                  dump_help();
                  return 0;
		}
              }
            }else{
              //unknown key
              dump_help();
              return 0;
            }
          }else{
            SKIP_BLANK(ptr);
            GET_UINT(temp_num,ptr,ptr2);
            if((ptr==ptr2)||(temp_num<1)){
	      //second chance if -n X
	      // then X is on next index
	      if(ISDIGIT(argv[index+1][0])){
		//BINGO
		ptr=&(argv[index+1][0]);
		GET_UINT(temp_num,ptr,ptr2);
		if(ptr==ptr2){//this should not happen
		  dump_help();
		  return 0;
		}
		index++;//to account for the value
	      }else{
		dump_help();
		return 0;
	      }
            }
            //register dos for sum
            n_dos++;
            ALLOC(tmp_idx,n_dos,int);
            IDX_CP(idx_dos,tmp_idx,n_dos-1);
            tmp_idx[n_dos-1]=temp_num;
            FREE(idx_dos);
            idx_dos=tmp_idx;
	    if(temp_num>max_n) max_n=temp_num;
          }
	  break;
	case 'u':
	  spin_up=TRUE;
	  break;
	case 'd':
	  spin_dn=TRUE;
	  break;
	case 'l':
	  //get the next word as label
	  ptr=&(argv[index][2]);
	  SKIP_BLANK(ptr);
	  STR_CLEAN(ptr);
	  STRDUP(ptr,label);
	  break;
	case 'v':
	  verbose=TRUE;
	  break;
	case 'h':
	default:
	  dump_help();
	  return 0;
	}
      }else{
        //just skip: we dont have parameter without switch
      }
      index++;
    }
  }
//first ensure there is a DOSCAR to read
  fp=fopen("DOSCAR","r");
  if(!fp){
    fprintf(stderr,"ERROR: opening the DOSCAR file!!!\n");
    fprintf(stderr,"Make sure DOSCAR file is present!\n");
    goto FAIL;
  }
  if(label!=NULL){
    wp=fopen(label,"w");
    if(!wp){
      fprintf(stderr,"ERROR: opening %s file for WRITE!!!\n",label);
      fprintf(stderr,"Make sure you have write permission!\n");
      goto FAIL;
    }
  }else{
    wp=stdout;/*should be alsways safe*/
  }
//THE READ
//--------
  READLINE(fp,line);//n_ions_total, n_ions, {0,1} partial DOS, unused
  ptr=&(line[0]);
  SKIP_BLANK(ptr);
  GET_UINT(total_atoms,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"1st line: total atoms");
  ptr2=ptr+1;SKIP_BLANK(ptr);
  if(!ISDIGIT(*ptr)) {
    fprintf(stderr,"DOSCAR (SKIPPED) READ ERROR: malformed 1st line!!!\n");
    //assume partial dos
    is_partial=TRUE;
  }else{
    if(*ptr=='0') is_partial=FALSE;
    else is_partial=TRUE;
  }
  //BAIL if partial request but not calculated
  if((n_atoms!=0)||(n_dos!=0)){
    if(!is_partial){
      fprintf(stderr,"ERROR: partial dos request but not included in DOSCAR!\n");
      fprintf(stderr,"PLEASE: check the LORBIT parameter for VASP!\n");
      goto FAIL;
    }
  }
  READLINE(fp,line);//volume, u, v, and w length, POTIM
  /*not used here*/
  READLINE(fp,line);// temperature TEBEG
  /*not used here*/
  READLINE(fp,line);//CAR
  /*not used here*/
  READLINE(fp,line);//SYSTEM tag
  ptr=&(line[0]);
  SKIP_BLANK(ptr);
  STR_CLEAN(ptr);
  fprintf(wp,"#SYSTEM=%s\n",line);
  READLINE(fp,line);//Emax, Emin, NEDOS, E_fermi, unused
  ptr=&(line[0]);
  SKIP_BLANK(ptr);
  //get Emax (ignored)
  GET_DOUBLE(ignore_d,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"6th line: Emax");
  ptr=ptr2+1;SKIP_BLANK(ptr);
  //get Emin (ignored)
  GET_DOUBLE(ignore_d,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"6th line: Emin");
  ptr=ptr2+1;SKIP_BLANK(ptr);
  //get NEDOS
  GET_UINT(total_dos,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"6th line: NEDOS");
  ptr=ptr2+1;SKIP_BLANK(ptr);
  //get E_fermi
  GET_DOUBLE(e_fermi,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"6th line: E_FERMI");
  //done
//complete intervals
//------------------
// atoms
if((from_a!=0)||(to_a!=0)){
  if(from_a<1) from_a=1;
  if(to_a<1) to_a=total_atoms;
  if(to_a>total_atoms) to_a=total_atoms;
  if(from_a>to_a) {
    fprintf(stderr,"atom interval error! (ignoring interval)\n");
    from_a=0;
    to_a=0;
  }
  if(to_a>max_a) max_a=to_a;
}
if(max_a>0){
  ALLOC(tmp_idx,(max_a+1),int);
  _jdx=0;
  for(index=1;index<=max_a;index++){
    if(n_atoms==0){
      tmp_idx[index]=1;
    }else{
      if((index>=from_a)&&(index<=to_a)) tmp_idx[index]=1;
      else
        for(_idx=0;_idx<n_atoms;_idx++)
          if(idx_atom[_idx]==index) tmp_idx[index]=1;
    }
    if(tmp_idx[index]==1){
      if(max_a<index) max_a=index;
      _jdx++;
    }
  }
  FREE(idx_atom);
  idx_atom=tmp_idx;
  n_atoms=_jdx;
}
// dos
if((from_n!=0)||(to_n!=0)){
  if(from_n<1) from_n=1;
  if(to_n<1) to_n=total_dos;
  if(to_n>total_atoms) to_n=total_dos;
  if(from_n>to_n){
    fprintf(stderr,"dos interval error! (ignoring interval)\n");
    from_n=0;
    to_n=0;
  }
  if(to_n>max_n) max_n=to_n;
}
if(max_n>0){
  ALLOC(tmp_idx,(max_n+1),int);
  _jdx=0;
  for(index=1;index<=max_n;index++){
    if((index>=from_n)&&(index<=to_n)) tmp_idx[index]=1;
    else
      for(_idx=0;_idx<n_dos;_idx++)
        if(idx_dos[_idx]==index) tmp_idx[index]=1;
    if(tmp_idx[index]==1){
      if(max_n<index) max_n=index;
      _jdx++;
    }
  }
  FREE(idx_dos);
  idx_dos=tmp_idx;
  n_dos=_jdx;
}
//GIVE SOME INFO
//--------------
if(verbose){
  fprintf(wp,"#PDOS called for %i atoms and %i dos\n",n_atoms,n_dos);
  if(n_atoms>0){
    fprintf(wp,"#DOS: ATOMS [ %i",idx_atom[0]);
    for(index=1;index<=max_a;index++) fprintf(wp,", %i",idx_atom[index]);
    fprintf(wp," ]\n");
  }
  if(n_dos>0){
    fprintf(wp,"#DOS: DOS [ %i",idx_dos[0]);
    for(index=1;index<=max_n;index++) fprintf(wp,", %i",idx_dos[index]);
    fprintf(wp," ]\n");
  }
}
//TODO: give spin up/down information
//READ TOTAL DOS
//--------------
  //initialize arrays
  ALLOC(energy,total_dos,double);
  ALLOC(dos_val,total_dos,double);
  READLINE(fp,line);//E, DOS, unused
  //On the first values we will determine if spin polarized was employed.
  //NOTE that on the first value core electron states are dumped so there
  //is a ridiculously big (& unphysical) DOS for the first energy value...
  //           !!! THEREFORE WE IGNORE THE FIRST DOS VALUE !!!
  ptr=&(line[0]);
  SKIP_BLANK(ptr);
  //get energy (ignored)
  GET_DOUBLE(temp_value,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"7th line: Energy");
  energy[0]=temp_value-e_fermi;
  ptr=ptr2+1;SKIP_BLANK(ptr);
  //get the DOS OR the spin up DOS value (ignored)
  GET_DOUBLE(ignore_d,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"7th line: DOS");
  ptr=ptr2+1;SKIP_BLANK(ptr);
  //get either the integration OR spin down value (ignored)
  GET_DOUBLE(ignore_d,ptr,ptr2);
  ASSERT_READ(ptr,ptr2,FAIL,"6th line: integral DOS (or DOS spin down)");
  ptr=ptr2+1;SKIP_BLANK(ptr);
  //try to get another value (which SET the have_spin on success)
  GET_DOUBLE(ignore_d,ptr,ptr2);
  if(ptr2==ptr) have_spin=FALSE;
  else have_spin=TRUE;
  //do we need title
/*
  if(have_spin){
    if((spin_up)&&(spin_dn)) fprintf(wp,"#Energy(eV) DOS_{up+down}(state/eV)\n");
    else if(spin_up) fprintf(wp,"#Energy(eV) DOS_{up}(state/eV)\n");
    else fprintf(wp,"#Energy(eV) DOS_{down}(state/eV)\n");
  }else{
    fprintf(wp,"#Energy(eV) DOS(state/eV)\n");
  }
*/
for(index=1;index<total_dos;index++){
  //other values -get it or loose it
  READLINE(fp,line);//E, DOS(s), unused(x2)
  if((n_dos == 0)&&(n_atoms==0)){
    ptr=&(line[0]);
    SKIP_BLANK(ptr);
    if(have_spin){
      //we will get up or down or sum
      GET_DOUBLE(temp_value,ptr,ptr2);//Energy
      ASSERT_READ(ptr,ptr2,FAIL,"total DOS: Energy");
      energy[index]=temp_value-e_fermi;
      ptr=ptr2+1;SKIP_BLANK(ptr);
      GET_DOUBLE(ignore_d,ptr,ptr2);//DOS up
      ASSERT_READ(ptr,ptr2,FAIL,"total DOS: DOS up spin");
      if(ignore_d < 0.0) ignore_d=0.0;//this is NOT COOP
      ptr=ptr2+1;SKIP_BLANK(ptr);
      GET_DOUBLE(temp_sum,ptr,ptr2);//DOS down
      ASSERT_READ(ptr,ptr2,FAIL,"total DOS: DOS down spin");
      if(temp_sum < 0.0) temp_sum=0.0;//this is NOT COOP
      if((spin_up)&&(spin_dn)){
	dos_val[index]=temp_sum+ignore_d;
      }else if(spin_up){
	dos_val[index]=ignore_d;
      }else{
	dos_val[index]=temp_sum;
      }  
    }else{
      GET_DOUBLE(temp_value,ptr,ptr2);//Energy
      ASSERT_READ(ptr,ptr2,FAIL,"total DOS: Energy");
      energy[index]=temp_value-e_fermi;
      ptr=ptr2+1;SKIP_BLANK(ptr);
      GET_DOUBLE(temp_sum,ptr,ptr2);//DOS
      if(temp_sum < 0.0) temp_sum=0.0;//this is NOT COOP
      dos_val[index]=temp_sum;
    }
  }else{
    //we don't need the total dos
    ptr=&(line[0]);
    SKIP_BLANK(ptr);
    //but we can register energy values
    GET_DOUBLE(temp_value,ptr,ptr2);//Energy
    ASSERT_READ(ptr,ptr2,FAIL,"total DOS: Energy");
    energy[index]=temp_value-e_fermi;
  }
}
/*now we reach END of TOTAL dos*/
if((n_dos==0)&&(n_atoms==0)){
  //in this case it is the end
  dump_dos(wp,energy,dos_val,total_dos);
  if(label!=NULL) fclose(wp);
  fclose(fp);
  FREE(line);
  FREE(label);
  FREE(idx_dos);
  FREE(idx_atom);
  FREE(energy);
  FREE(dos_val);
  return 0;
}
//----------------
//READ PARTIAL DOS
//----------------
//reprocess n_atoms==0
if(n_atoms==0){
  ALLOC(tmp_idx,(total_atoms+1),int);
  for(_idx=1;_idx<=total_atoms;_idx++) tmp_idx[_idx]=1;
  FREE(idx_atom);
  idx_atom=tmp_idx;
  n_atoms=total_atoms;
  max_a=n_atoms;
  if(verbose){
    fprintf(wp,"#DOS: ATOMS [ %i",idx_atom[0]);
    for(index=1;index<=max_a;index++) fprintf(wp,", %i",idx_atom[index]);
    fprintf(wp," ]\n");
  }
}
//READ LOOP
//---------
for(index=1;index<=max_a;index++){
//if atom is not among projection, skip all reading
  if(idx_atom[index]==1){
    //we need to do some integration on this atom
    //ignore header
    READLINE(fp,line);//Emax, Emin, NEDOS, E_fermi, unused
    _idx=0;
    //Similarly to total DOS, the first value is ridiculously big (& unphysical)
    // !!! WHICH IS WHY WE IGNORE IT !!!
    READLINE(fp,line);
    //BUT we can use it to
    //reprocess n_dos==0
    if(n_dos==0){
      //we are going through this line to know how many DOS are projected
      ptr=&(line[0]);
      GET_DOUBLE(temp_value,ptr,ptr2);//energy
      ASSERT_READ(ptr,ptr2,FAIL,"partial DOS: Energy");
      _jdx=0;
      do{
	ptr=ptr2+1;SKIP_BLANK(ptr);
	GET_DOUBLE(ignore_d,ptr,ptr2);//value(up)
	if((ptr!=ptr2)&&(have_spin)){
	  ptr=ptr2+1;SKIP_BLANK(ptr);
	  GET_DOUBLE(ignore_d,ptr,ptr2);//value(down)
	}
	_jdx++;
      }while(ptr2!=ptr);
      n_dos=_jdx-1;
      ALLOC(tmp_idx,(n_dos+1),int);
      for(_jdx=1;_jdx<=n_dos;_jdx++) tmp_idx[_jdx]=1;
      FREE(idx_dos);
      idx_dos=tmp_idx;
      max_n=n_dos;
      if(verbose){
	fprintf(wp,"#DOS: DOS [ %i",idx_dos[0]);
	for(index=1;index<=max_n;index++) fprintf(wp,", %i",idx_dos[index]);
	fprintf(wp," ]\n");
      }
    }
    _idx++;
    READLINE(fp,line);
    _idx++;
    do{
      //sum over all DOS
      ptr=&(line[0]);
      GET_DOUBLE(temp_value,ptr,ptr2);//energy
      ASSERT_READ(ptr,ptr2,FAIL,"partial DOS: Energy");
      //from DOS#1 to DOS#n_dos
      //sum if DOS is in idx_dos
      _jdx=1;
      if(have_spin){
        temp_sum=0.0;
	do{
	  ptr=ptr2+1;SKIP_BLANK(ptr);
	  GET_DOUBLE(temp_value,ptr,ptr2);//value up
	  ASSERT_READ(ptr,ptr2,FAIL,"partial DOS: Value");
	  GET_DOUBLE(ignore_d,ptr,ptr2);//value down
	  ASSERT_READ(ptr,ptr2,FAIL,"partial DOS: Value");
	  //values are interlaced
	  if(idx_dos[_jdx]==1) {
	    if((spin_up)&&(spin_dn)){
	      temp_sum += ignore_d + temp_value;
	    }else if(spin_up){
	      temp_sum += temp_value;
	    }else{
	      temp_sum += ignore_d;
	    }
	  }
	  _jdx++;
	}while(_jdx<=max_n);
	if(temp_sum < 0.0) temp_sum=0.0;//this is NOT COOP
      }else{
	temp_sum=0.0;
        do{
	  ptr=ptr2+1;SKIP_BLANK(ptr);
          GET_DOUBLE(ignore_d,ptr,ptr2);//value
          ASSERT_READ(ptr,ptr2,FAIL,"partial DOS: Value");
          if(idx_dos[_jdx]==1) temp_sum += ignore_d;
	  _jdx++;
        }while(_jdx<=max_n);
	if(temp_sum < 0.0) temp_sum=0.0;//this is NOT COOP
      }
      dos_val[_idx]+=temp_sum;
      READLINE(fp,line);
      _idx++;
    }while((!feof(fp))&&(_idx<total_dos));
  }else{
    //no integration needed
    //just READ-SKIP
    READLINE(fp,line);//HEADER
    _idx=0;
    do{
      READLINE(fp,line);
      _idx++;
    }while((!feof(fp))&&(_idx<total_dos));
  }
}
//END
  dump_dos(wp,energy,dos_val,total_dos);
  if(fp!=NULL) fclose(fp);
  if(label!=NULL){
    if(wp!=NULL) fclose(wp);
  }
  FREE(line);
  FREE(label);
  FREE(idx_dos);
  FREE(idx_atom);
  FREE(energy);
  FREE(dos_val);
  return 0;
FAIL:
  //cleanup
  if(fp!=NULL) fclose(fp);
  if(label!=NULL){
    if(wp!=NULL) fclose(wp);
  }
  FREE(line);
  FREE(label);
  FREE(idx_dos);
  FREE(idx_atom);
  FREE(energy);
  FREE(dos_val);
  return 1;
}
