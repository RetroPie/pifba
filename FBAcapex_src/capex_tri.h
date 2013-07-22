/*  CAPEX for GP2X

    Copyright (C) 2007  JyCet

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//algo de tri alphabetique
void tri_alphabeticAZ(unsigned int minimun, unsigned int maximum)
{
	//unsigned int longueur;
	unsigned int tampon,ligne1,ligne2;
	unsigned int min;
	
	//tri des dossiers en premier	
	for (ligne1=minimun;ligne1<maximum-1;++ligne1){
		min=ligne1;
		for (ligne2=ligne1+1;ligne2<maximum;++ligne2){
			if (strcmp( data.name[listing_tri[0][ligne2]] , data.name[listing_tri[0][min]] ) <0 ) {
				min=ligne2;
			}
		}
		if (min!=ligne1){
			tampon=listing_tri[0][ligne1];
			listing_tri[0][ligne1]=listing_tri[0][min];
			listing_tri[0][min]=tampon;
		}
	}
}
