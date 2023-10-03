#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <zip.h>
#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <cairo/cairo-ft.h>

typedef struct{
	uint32_t x[4];
} xorshift128_state;

uint32_t xorshift128(xorshift128_state *state){

	uint32_t t  = state->x[3];
	uint32_t s  = state->x[0];
	state->x[3] = state->x[2];
	state->x[2] = state->x[1];
	state->x[1] = s;

	t ^= t << 11;
	t ^= t >> 8;
	return state->x[0] = t ^ s ^ (s >> 19);
}

typedef struct{
	char *current_position;
	char *end_of_array;
} closure_t;

static cairo_status_t write_svg_to_buffer(
	void *in_closure, 
	const unsigned char *data,
	unsigned int length
){
closure_t *closure = (closure_t*) in_closure;

if ((closure->current_position + length) > (closure->end_of_array))
return CAIRO_STATUS_WRITE_ERROR;

memcpy (closure->current_position, data, length);
closure->current_position += length;

return CAIRO_STATUS_SUCCESS;
}

void get_random_id(xorshift128_state *state,char *id){
	xorshift128(state);
	sprintf(
		id,"%08x%08x%08x%08x",
		state->x[0],state->x[1],state->x[2],state->x[3]
	);
	strcat(id,"\0");
}

typedef struct{
	bool number;
	bool letter;
	bool punctual;
	bool other;
	char output[512];
	char fontfile[512];
} option_t;


char* generate_list(size_t *list_len, option_t *opt){
	size_t size = 20;
	char *list  = malloc(size);
	char *listP = list;
	*listP = ' ';
	listP++;
	if(opt->number){
		for(char i='1';i<='9';i++){
			*listP = i;
			listP++;
		}
		*listP='0';
		listP++;
	}
	
	if(opt->letter){
		for(char i='A';i<='Z';i++){
			*listP = i;
			listP++;
			if(listP-list>=size){
				size+=10;
				list = realloc(list,size);
			}
		}
		for(char i='a';i<='z';i++){
			*listP = i;
			listP++;
			if(listP-list>=size){
				size+=10;
				list = realloc(list,size);
			}
		}
	}
	
	if(opt->punctual){
		char *i=".!?\'\"`:;\0";
		for(;*i!=0;i++){
			*listP =*i;
			listP++;
			if(listP-list>=size){
				size+=10;
				list = realloc(list,size);
			}
		}
	}
	
	if(opt->other){
		char *i="_@#$%&^|/\\>=<()[]{}\0";
		for(;*i!=0;i++){
			*listP = *i;
			listP++;
			if(listP-list>=size){
				size+=10;
				list = realloc(list,size);
			}
		}
	}
	
	*listP=0;
	*list_len = strlen(list);
	return list;
}

void help(int exit_code){
	puts(
		"Usage:\n"
		"   scratchfont [OPTIONS]... font.ttf font.sprite3\n\n"
		"Options:\n"
		"   -h\t  Print this page\n"
		"   -n\t  To convert the numbers ([0-9])\n"
		"   -l\t  To convert the letters ([A-Z] [a-z])\n"
		"   -p\t  To convert the punctuations(.!?\'\"`:;)\n"
		"   -o\t  To convert the other ASCII text(_@#$%&^|/\\>=<()[]{})\n"
		"   -w\t  Alias for -l\n"

	);
	exit(exit_code);
}

void argument(char *str, option_t *opt){
	while (*str != '\0') {
			switch (*str++) {
				case 'n': opt->number = 1; break;
				case 'l': opt->letter = 1; break;
				case 'w': opt->letter = 1; break;
				case 'p': opt->punctual = 1; break;
				case 'o': opt->other = 1; break;
				case 'h': help(EXIT_SUCCESS); break;
		}
	}
}

option_t get_options(int argc, char *argv[]){
	option_t opt;
	memset(&opt, 0, sizeof(option_t));
	for (int i = 1; i < argc; ++i) {
	  if (*argv[i] == '-') {
	    argument(argv[i],&opt);
		}
		else{
			if(strlen(opt.fontfile)==0){
				strcpy(opt.fontfile,argv[i]);
			}	
			else{
				strcpy(opt.output,argv[i]);
			}
		}
	}
	
	if(!opt.number &&
		 !opt.letter &&
		 !opt.punctual &&
		 !opt.other 
		){
		opt.number=1;
		opt.letter=1;
	}
	
	return opt;
}

int main(int argc, char *argv[]){
	
	if(argc==1){
		fputs("Error: Not enough arguments\n",stderr);
		help(EXIT_FAILURE);
	}

	option_t opt = get_options(argc,argv);
	//Checks if output has been set
	if(strlen(opt.output)==0){
		strcpy(opt.output,"font.sprite3");
	}
	
	if(*opt.fontfile==0){
		fputs("Error: fontfile is not set\n",stderr);
		help(EXIT_FAILURE);
	}
	
 	char *list;
  size_t list_len;
  
  list = generate_list(&list_len,&opt);
	
	#if 1
 
	FT_Library library;
  FT_Error status;
  FT_Face face;

  cairo_t* cr;
  cairo_surface_t* surface;
  cairo_font_face_t* cface;
  cairo_text_extents_t extents;
  
  
  int size = 25;


  FT_Init_FreeType(&library);
  
  status = FT_New_Face(library, opt.fontfile, 0, &face);
  if (status != 0) {
		fprintf(stderr, "Error: code=%d Failed opening %s.\n", status, opt.fontfile);
		exit(EXIT_FAILURE);
  }
  
  closure_t closure;
	char id[36];
	char buffer[1024*8];
	char costume[1024*32];
	char spritedata[1024*34];
	char filename[36];
  char temp[128];
	
  zip_t *zip;
  zip_source_t *so;
  
  if(access(opt.output, F_OK) == 0){
  	remove(opt.output);
  }
	xorshift128_state state;
	state.x[3]=178338837;
	state.x[2]=738557538;
	state.x[1]=588489310;
	state.x[0]=346790742;
  
	for(const char *listP=list;*listP!=0;listP++){
		if(strlen(costume)!=0)
			strcat(costume,",");
  	zip = zip_open(opt.output,ZIP_CREATE,NULL);
		memset(buffer,0,sizeof(buffer));
		memset(id,0,sizeof(id));
		get_random_id(&state,id);
		
		char ch[3] = {*listP,0};
		char chsafe[3] = {*listP,0};
		if(*listP=='\\'||*listP=='"'){
			char *tmp = strdup(ch);
			strcpy(chsafe,"\\");
			strcat(chsafe,tmp);
			free(tmp);
		}
	
		surface = cairo_svg_surface_create_for_stream(NULL,NULL,0,0);
		cr = cairo_create(surface);

		cface = cairo_ft_font_face_create_for_ft_face(face,0);
		cairo_set_font_face(cr,cface);
		cairo_set_font_size(cr,size);
		cairo_text_extents(cr,ch,&extents);
		double width = extents.width;
		double height = extents.height;

		cairo_surface_destroy(surface);
		cairo_destroy(cr);

		//Resizing the surface
		closure.current_position = buffer;
		closure.end_of_array = buffer + sizeof (buffer);
		surface = cairo_svg_surface_create_for_stream(
			write_svg_to_buffer,&closure,
			width+2,height+2
		);

		cairo_svg_surface_set_document_unit(surface,CAIRO_SVG_UNIT_PX);

		cr = cairo_create(surface);
		cairo_set_font_face(cr,cface);
		cairo_set_font_size(cr,size);

		cairo_move_to(cr,-extents.x_bearing+1,-extents.y_bearing+1);
		cairo_set_source_rgb(cr,0,0,0);
		cairo_show_text(cr,ch);

		cairo_surface_flush(surface);
		cairo_surface_finish(surface);
		strcpy(filename,id);
		strcat(filename,".svg");
		
		so = zip_source_buffer(zip,buffer,strlen(buffer)*sizeof(char),0);
	  zip_file_add(zip,filename, so, ZIP_FL_OVERWRITE);
  	zip_close(zip);
		memset(filename,0,sizeof(filename));
  	
		sprintf(temp,"{\"assetId\":\"%s\",\"name\":\"%s\",\"bitmapResolution\":1,\"md5ext\":\"%s.svg\",\"dataFormat\":\"svg\",\"rotationCenterX\":1,\"rotationCenterY\":%f}",id,chsafe,id,height+1);
		strcat(costume,temp);
		// printf("%c\n",*listP);
		memset(temp,0,sizeof(temp));
	}
	
	free(list);
  zip = zip_open(opt.output,ZIP_CREATE,NULL);
	sprintf(spritedata,"{\"isStage\":false,\"name\":\"Font\",\"variables\":{},\"lists\":{},\"broadcasts\":{},\"blocks\":{},\"comments\":{},\"currentCostume\":0,\"costumes\":[%s],\"sounds\":[],\"volume\":100,\"visible\":true,\"x\":0,\"y\":0,\"size\":100,\"direcfaceion\":90,\"draggable\":false,\"rotationStyle\":\"all around\"}",costume);
	
  so=zip_source_buffer(zip,spritedata,strlen(spritedata)*sizeof(char),0);
  zip_file_add(zip, "sprite.json", so, ZIP_FL_OVERWRITE);
  zip_close(zip);
  
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  FT_Done_Face(face);
  FT_Done_FreeType(library);
  #endif
	return 0;
}
