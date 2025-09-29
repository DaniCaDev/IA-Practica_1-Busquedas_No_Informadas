# =========================
#  Proyecto: Busquedas no informadas (BFS/DFS)
#  C++17 - Estilo Google - Makefile con dependencias
# =========================

# Compilador y flags
CXX       := g++
STD       := gnu++23
WARNFLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
OPT_REL   := -O2
OPT_DBG   := -O0 -g3
DEPFLAGS  := -MMD -MP
# Descomenta si quieres sanitizers en debug:
# SANITIZERS := -fsanitize=address,undefined

# Directorios
INCDIR    := include
SRCDIR    := src
BUILDDIR  := build
BINDIR    := bin
TARGET    := $(BINDIR)/busquedas

# Fuentes / objetos
SRCS      := $(wildcard $(SRCDIR)/*.cc)
OBJS      := $(patsubst $(SRCDIR)/%.cc,$(BUILDDIR)/%.o,$(SRCS))
DEPS      := $(OBJS:.o=.d)

# Flags
CPPFLAGS  := -I$(INCDIR)
CXXFLAGS  := -std=$(STD) $(WARNFLAGS) $(DEPFLAGS)
LDFLAGS   :=

# =========================
# Reglas
# =========================

.PHONY: all release debug clean distclean run help dirs

all: release

release: CXXFLAGS += $(OPT_REL)
release: dirs $(TARGET)
	@echo ">> Listo (release): $(TARGET)"

debug: CXXFLAGS += $(OPT_DBG) $(SANITIZERS)
debug: LDFLAGS  += $(SANITIZERS)
debug: dirs $(TARGET)
	@echo ">> Listo (debug): $(TARGET)"

dirs:
	@mkdir -p $(BUILDDIR) $(BINDIR)

# Enlazado final
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compilación con dependencias automáticas
$(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Ejecutar con parámetros (sobrescribe ARGS en línea de comandos)
# Ejemplo:
#  make run ARGS="--input data/Grafo1.txt --origin 1 --dest 4 --strategy bfs --output resultado.txt"
run: release
	@$(TARGET) $(ARGS)

clean:
	@echo ">> Limpiando objetos y dependencias..."
	@rm -rf $(BUILDDIR)

distclean: clean
	@echo ">> Limpiando binarios..."
	@rm -rf $(BINDIR)

help:
	@echo "Targets:"
	@echo "  make / make release   -> build optimizado"
	@echo "  make debug            -> build debug (opcional con sanitizers)"
	@echo "  make run ARGS='...'   -> ejecutar con argumentos"
	@echo "  make clean            -> borra objetos"
	@echo "  make distclean        -> borra objetos y binarios"

# Incluir archivos .d generados por -MMD -MP
-include $(DEPS)
