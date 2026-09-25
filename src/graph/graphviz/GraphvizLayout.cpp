// Implements Graphviz context management and layout.
#include "graph/graphviz/GraphvizLayout.hpp"

#include "graph/graphviz/detail/LayoutExtractor.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

extern "C"
{
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>
}

#ifdef REGEXTHESIS_STATIC_GRAPHVIZ_PLUGINS
extern "C"
{
    extern gvplugin_library_t gvplugin_core_LTX_library;
    extern gvplugin_library_t gvplugin_dot_layout_LTX_library;
}
#endif

namespace graph::graphviz
{
    namespace
    {
        // Releases a Graphviz context through a smart pointer.
        struct ContextDeleter
        {
            // Frees a Graphviz context.
            void operator()(GVC_t* context) const
            {
                gvFreeContext(context);
            }
        };

        // Closes a Graphviz graph through a smart pointer.
        struct GraphDeleter
        {
            // Closes a Graphviz graph.
            void operator()(Agraph_t* graph) const
            {
                agclose(graph);
            }
        };

        // Releases Graphviz-rendered data through a smart pointer.
        struct RenderDataDeleter
        {
            // Frees a buffer returned by Graphviz rendering.
            void operator()(char* data) const
            {
                gvFreeRenderData(data);
            }
        };

        // Guarantees `gvFreeLayout` runs before the context and graph are released.
        class LayoutGuard
        {
        public:
            // Borrows the context and graph whose active layout it guards.
            LayoutGuard(GVC_t* context, Agraph_t* graph) : context_(context), graph_(graph)
            {
            }

            // Releases Graphviz's layout allocations.
            ~LayoutGuard()
            {
                gvFreeLayout(context_, graph_);
            }

            // Layout guards represent unique cleanup responsibility and cannot be copied.
            LayoutGuard(const LayoutGuard&) = delete;
            // Layout guards represent unique cleanup responsibility and cannot be copied.
            LayoutGuard& operator=(const LayoutGuard&) = delete;

        private:
            GVC_t* context_;
            Agraph_t* graph_;
        };

        // Owning Graphviz context handle.
        using Context = std::unique_ptr<GVC_t, ContextDeleter>;
        // Owning Graphviz graph handle.
        using Graph = std::unique_ptr<Agraph_t, GraphDeleter>;
        // Owning Graphviz render-data handle.
        using RenderData = std::unique_ptr<char, RenderDataDeleter>;

        // Creates a dynamic or statically registered Graphviz context.
        [[nodiscard]] GVC_t* create_context()
        {
#ifdef REGEXTHESIS_STATIC_GRAPHVIZ_PLUGINS
            static lt_symlist_t plugins[] = {
                {"gvplugin_dot_layout_LTX_library", &gvplugin_dot_layout_LTX_library},
                {"gvplugin_core_LTX_library", &gvplugin_core_LTX_library},
                {nullptr, nullptr}
            };
            return gvContextPlugins(plugins, 0);
#else
            return gvContext();
#endif
        }

        // Sets a graph attribute using the value as its default.
        void set_layout_attribute(Agraph_t* graph, const char* name, const char* value)
        {
            agsafeset(
                graph, const_cast<char*>(name), const_cast<char*>(value), const_cast<char*>(value)
            );
        }

        // Runs one Graphviz engine and extracts its populated geometry attributes.
        [[nodiscard]] LayoutResult run_layout(Context& context, Graph& graph, const char* engine)
        {
            if (gvLayout(context.get(), graph.get(), engine) != 0)
            {
                return LayoutError{
                    LayoutErrorCode::LayoutFailed, "Graphviz could not compute a graph layout."
                };
            }
            const LayoutGuard layout_guard(context.get(), graph.get());

            char* raw_rendered_dot = nullptr;
            std::size_t rendered_length = 0;
            // Rendering to DOT materializes edge splines and label positions as attributes.
            const int render_status = gvRenderData(
                context.get(), graph.get(), "dot", &raw_rendered_dot, &rendered_length
            );
            const RenderData rendered_dot(raw_rendered_dot);
            if (render_status != 0)
            {
                return LayoutError{
                    LayoutErrorCode::RenderFailed, "Graphviz could not expose the computed layout."
                };
            }

            return detail::extract_layout(graph.get());
        }

    }

    LayoutResult compute_layout(const std::string_view dot)
    {
        if (dot.empty())
        {
            return LayoutError{LayoutErrorCode::EmptyInput, "DOT input is empty."};
        }

        Context context(create_context());
        if (!context)
        {
            return LayoutError{
                LayoutErrorCode::ContextInitializationFailed, "Could not initialize Graphviz."
            };
        }

        const std::string terminated_dot(dot);
        Graph graph(agmemread(terminated_dot.c_str()));
        if (!graph)
        {
            return LayoutError{
                LayoutErrorCode::InvalidDot, "Graphviz could not parse the generated DOT graph."
            };
        }

        set_layout_attribute(graph.get(), "splines", "true");
        set_layout_attribute(graph.get(), "nodesep", "0.75");
        set_layout_attribute(graph.get(), "ranksep", "0.75");
        set_layout_attribute(graph.get(), "pad", "0.2");

        return run_layout(context, graph, "dot");
    }

}
