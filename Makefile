.PHONY: clean fclean re all libjson

NAME 		   =   serv

CXX            =   clang++
# CPPFLAGS     =   -Wall -Wextra -Werror  -Wc++11-extensions -std=c++98 -g -fstandalone-debug # -Wpedantic
CPPFLAGS       =   -Wall -Wextra -Werror -Wc++11-extensions -std=c++98 -g

SOURCEFILES     =	main.cpp \
				    Server.cpp \
					Request.cpp \
					ReadSock.cpp \
					Request.cpp \
					ServerBlock.cpp \
					Utils.cpp \
					CRC.cpp \
					CGI.cpp \
					Redirect.cpp \
					ValidHeaders.cpp \
					Client.cpp \
					Logger.cpp \
					Response.cpp \
					Header.cpp \
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
	@if ! [ "$(ls $(LIBJSONFOLDER))" ] ; then git submodule update --init; fi
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

