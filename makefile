# =========================
#  Proyecto: Busquedas no informadas (BFS/DFS)
#  Estilo: Google (flags estrictos)
# =========================

# --- Configuración general ---
CXX       := g++
STD       := gnu++17
WARNFLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
OPT_REL   := -O2
OPT_DBG   := -O0 -g3
DEPFLAGS  := -MMD -MP

# --- Rutas ---
INCDIR    := include
SRCDIR    := src
BUILDDIR  := build
BINDIR    := bin
TARGET    := $(BINDIR)/busquedas

# --- Fuentes y objetos ---
SRCS      := $(wildcard $(SRCDIR)/*.cc)
OBJS      := $(patsubst $(SRCDIR)/%.cc,$(BUILDDIR)/%.o,$(SRCS))
DEPS      := $(OBJS:.o=.d)

# --- Flags de compilación y enlace ---
CPPFLAGS  := -I$(INCDIR)
CXXFLAGS  := -std=$(STD) $(WARNFLAGS) $(DEPFLAGS)
LDFLAGS   :=

# =========================
#        Reglas
# =========================

# Por defecto: build release
.PHONY: all
all: release

# Build release (optimizado)
.PHONY: release
release: CXXFLAGS += $(OPT_REL)
release: dirs $(TARGET)
	@echo ">> Listo (release): $(TARGET)"

# Build debug (con símbolos)
.PHONY: debug
debug: CXXFLAGS += $(OPT_DBG)
debug: dirs $(TARGET)
	@echo ">> Listo (debug): $(TARGET)"

# Crear carpetas
.PHONY: dirs
dirs:
	@mkdir -p $(BUILDDIR) $(BINDIR)

# Enlazado final
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compilación incremental con dependencias
$(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Limpiar
.PHONY: clean
clean:
	@echo ">> Limpiando objetos y dependencias..."
	@rm -rf $(BUILDDIR)

.PHONY: distclean
distclean: clean
	@echo ">> Limpiando binarios..."
	@rm -rf $(BINDIR)

# Ejecutar con parámetros (puedes sobreescribir ARGS en línea de comandos)
# Ejemplo:
#   make run ARGS="--input datos.txt --origin 1 --dest 4 --strategy bfs --output resultado.txt"
.PHONY: run
run: release
	@$(TARGET) $(ARGS)

# Objetivo de ayuda
.PHONY: help
help:
	@echo "Targets disponibles:"
	@echo "  make            -> build release (por defecto)"
	@echo "  make release    -> build optimizado"
	@echo "  make debug      -> build con símbolos de depuración"
	@echo "  make run ARGS='--input ... --origin ... --dest ... --strategy bfs|dfs --output ...'"
	@echo "  make clean      -> borra objetos intermedios"
	@echo "  make distclean  -> borra objetos y binarios"

# Incluir dependencias generadas automáticamente
-include $(DEPS)
