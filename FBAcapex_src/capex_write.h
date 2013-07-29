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

void write_cfg()
{
	FILE *fp;

	fp = fopen("capex.cfg", "w");

	fputs("# CAPEX config file v1.1\n",fp);
	fputs("\n",fp);
	
	fputs("# FBA2X OPTIONS\n",fp);
	fputs("\n",fp);
	sprintf((char*)g_string, "fba2x_sound %d\n",options.sound);
	fputs(g_string,fp);
	sprintf((char*)g_string, "fba2x_samplerate %d\n",options.samplerate);
	fputs(g_string,fp);
//	sprintf((char*)g_string, "fba2x_rescale %d\n",options.rescale);
//	fputs(g_string,fp);
	sprintf((char*)g_string, "fba2x_showfps %d\n",options.showfps);
	fputs(g_string,fp);

	fputs("\n",fp);
	
	fputs("# CAPEX OPTIONS\n",fp);
	fputs("\n",fp);
	sprintf((char*)g_string, "capex_list %d\n",capex.list);
	fputs(g_string,fp);
	sprintf((char*)g_string, "capex_sely %d\n",capex.sely);
	fputs(g_string,fp);
	sprintf((char*)g_string, "capex_selnum %d\n",capex.selnum);
	fputs(g_string,fp);
	sprintf((char*)g_string, "capex_seloffset_num %d\n",capex.seloffset_num);
	fputs(g_string,fp);

	fclose(fp);
}

