.PHONY: clean fclean re all libjson

NAME 			= 	serv

CXX             =   clang++
CPPFLAGS        =   -g -fstandalone-debug -std=c++98
# CPPFLAGS        =   -Wall -Wextra -Werror -g -std=c++98
# CPPFLAGS       =   -Wall -Wextra -Werror -g -Wpedantic -std=c++98

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
					Response.cpp \
					ResponseContType.cpp \
					ResponseErrCode.cpp \
					Config.cpp \
					Location.cpp
				
LIBJSONFOLDER = json-parser
LIBJSONINCLUDE = ./$(LIBJSONFOLDER)/src
LIBJSONFLAGS = -ljson -L ./$(LIBJSONFOLDER) -I ${LIBJSONINCLUDE} 

PAGES           =   pages/
SOURCEFOLDER    =   src/
OSOURCEFOLDER   =   .obj/
INCLUDEFOLDER   =   include/

SOURCE          =   $(addprefix $(SOURCEFOLDER), $(SOURCEFILES))
OSOURCE         =   $(addprefix $(OSOURCEFOLDER), $(SOURCEFILES:.cpp=.o))

all: libjson objdir $(NAME)

objdir:
	@if ! [ -d ${OSOURCEFOLDER} ] ; then mkdir ${OSOURCEFOLDER} ; fi

$(OSOURCEFOLDER)%.o: $(SOURCEFOLDER)%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ -I $(INCLUDEFOLDER) -I $(LIBJSONINCLUDE)

libjson:
# --remote takes the last commit ?
	@if ! [ "$(ls $(LIBJSONFOLDER))" ] ; then git submodule update --remote; fi
	$(MAKE) -C $(LIBJSONFOLDER) all

$(NAME): $(OSOURCE)
	$(CXX) $(CPPFLAGS) $^ -o $(NAME) $(LIBJSONFLAGS)

clean:
	$(MAKE) -C $(LIBJSONFOLDER) clean
	rm -rf $(OSOURCEFOLDER) YoupiBanane

fclean: clean
	$(MAKE) -C $(LIBJSONFOLDER) fclean
	rm -rf $(NAME)

re: fclean all

# for tests

YoupiBanane:
				mkdir -p YoupiBanane/nop YoupiBanane/Yeah
				touch ./YoupiBanane/youpi.bad_extension ./YoupiBanane/youpi.bla
				touch ./YoupiBanane/nop/youpi.bad_extension ./YoupiBanane/nop/other.pouic
				touch ./YoupiBanane/Yeah/not_happy.bad_extension

