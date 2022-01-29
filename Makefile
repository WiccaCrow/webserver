.PHONY: clean fclean re all libjson

NAME 			= 	serv

CXX             =   clang++
CPPFLAGS        =   -Wall -Wextra -Werror -g -std=c++98
#CPPFLAGS       =   -Wall -Wextra -Werror -g -Wpedantic -std=c++98

SOURCEFILES     =	main.cpp \
				    Server.cpp \
					Request.cpp \
					ReadSock.cpp \
					Request.cpp \
					ServerBlock.cpp \
					Utils.cpp \
					CRC.cpp \
					ValidateHeaders.cpp \
					Client.cpp \
					Logger.cpp \
					Response.cpp
				
LIBJSONFOLDER   =   #json-parser
LIBJSONINCLUDE  =   #-I ./$(LIBJSONFOLDER)/src
LIBJSONFLAGS    =   #-ljson -L ./$(LIBJSONFOLDER) ${LIBJSONINCLUDE}

PAGES           =   pages/
SOURCEFOLDER    =   src/
OSOURCEFOLDER   =   .obj/
INCLUDEFOLDER   =   include/

SOURCE          =   $(addprefix $(SOURCEFOLDER), $(SOURCEFILES))
OSOURCE         =   $(addprefix $(OSOURCEFOLDER), $(SOURCEFILES:.cpp=.o))

all: objdir $(NAME)

objdir:
	@if ! [ -d ${OSOURCEFOLDER} ] ; then mkdir ${OSOURCEFOLDER} ; fi

$(OSOURCEFOLDER)%.o: $(SOURCEFOLDER)%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ -I $(INCLUDEFOLDER) $(LIBJSONINCLUDE)

libjson:
	@if ! [ "$(ls $(LIBJSONFOLDER))" ] ; then git submodule update --init; fi
	$(MAKE) -C $(LIBJSONFOLDER) all

$(NAME): $(OSOURCE)
	$(CXX) $(CPPFLAGS) $^ -o $(NAME) $(LIBJSONFLAGS)

clean:
	# $(MAKE) -C $(LIBJSONFOLDER) clean
	rm -rf $(OSOURCEFOLDER)

fclean: clean
	# $(MAKE) -C $(LIBJSONFOLDER) fclean
	rm -rf $(NAME)

re: fclean all

