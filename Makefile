.PHONY: clean fclean re all libjson makedir

NAME = webserv

###################################################################################
#                               Compiler & Flags                                  #
###################################################################################

CXX       =   clang++
CPPFLAGS  =   -Wall -Wextra -Werror -std=c++98 -g
LDFLAGS   =   -lpthread

ifeq ($(shell uname), Linux)
	LDFLAGS    += -lcrypt
	CPPFLAGS   += -fstandalone-debug
endif

###################################################################################
#                              Directories & Files                                #
###################################################################################

SRCS_DIR     = src
OBJS_DIR     = .obj
DEPS_DIR     = .deps
INCLUDE_DIR  = include
LOGS_DIR     = logs
DFLT_DIR	 = default

SRCS     =  Auth.cpp                Location.cpp            SHA1.cpp           \
            Base64.cpp              Logger.cpp              Server.cpp         \
            CGI.cpp                 Time.cpp                ServerBlock.cpp    \
            CRC.cpp                 Redirect.cpp            StatusLines.cpp    \
            Client.cpp              Request.cpp             URI.cpp            \
            Config.cpp              RequestHeader.cpp       Utils.cpp          \
            ErrorResponses.cpp      Response.cpp            main.cpp           \
            Header.cpp              ResponseContType.cpp    Range.cpp		   \
            HeaderNames.cpp         ResponseHeader.cpp      Worker.cpp         \
			AClient.cpp

OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.cpp=.o))
DEPS = $(addprefix $(DEPS_DIR)/, $(SRCS:.cpp=.d))

###################################################################################
#                                   Libjson                                       #
###################################################################################

LIBJSONDIR = json-parser
LIBJSONINCLUDE = ./${LIBJSONDIR}/src
LIBJSONFLAGS = -ljson -L ./${LIBJSONDIR} -I ${LIBJSONINCLUDE} 

###################################################################################
#                                    Cycle                                        #
###################################################################################

all: libjson makedir $(NAME)

libjson:
	@if ! [ "$(ls $(LIBJSONDIR))" ] ; then git submodule update --init; fi
	$(MAKE) -C $(LIBJSONDIR) all

makedir:
	@if ! [ -d ${OBJS_DIR} ] ; then mkdir ${OBJS_DIR} ; fi
	@if ! [ -d ${DEPS_DIR} ] ; then mkdir ${DEPS_DIR} ; fi
	@if ! [ -d ${LOGS_DIR} ] ; then mkdir ${LOGS_DIR} ; fi


$(NAME): $(OBJS)
	$(CXX) $(CPPFLAGS) $^ -o $(NAME) $(LIBJSONFLAGS) $(LDFLAGS)

-include $(DEPS)
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ -I $(INCLUDE_DIR) -I $(LIBJSONINCLUDE) \
        -MMD -MF $(patsubst ${OBJS_DIR}/%.o,${DEPS_DIR}/%.d,$@) -D LOGS_DIR=\"${LOGS_DIR}\"

clean:
	rm -rf ${DEPS_DIR} ${OBJS_DIR} ${LOGS_DIR} YoupiBanane

fclean: clean
	rm -rf $(NAME)

re: fclean all

###################################################################################
#                                    Tests                                        #
###################################################################################

cgi:
	gcc ./pages/site/cgi/printenv.c -o ./pages/site/cgi/printenv.cgi

YoupiBanane:
	mkdir -p YoupiBanane/nop YoupiBanane/Yeah
	touch ./YoupiBanane/youpi.bad_extension ./YoupiBanane/youpi.bla
	touch ./YoupiBanane/nop/youpi.bad_extension ./YoupiBanane/nop/other.pouic
	touch ./YoupiBanane/Yeah/not_happy.bad_extension

