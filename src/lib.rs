use nih_plug::prelude::*;
use nih_plug_egui::{create_egui_editor, EguiState};
use std::num::NonZeroU32;
use std::sync::Arc;

mod ui;
use ui::GradientKnobUiState;

//==============================================================================
// Parameters
//==============================================================================
#[derive(Params)]
struct GradientKnobParams {
    /// The 0-100 value driven by the knob. Automatable from the host.
    #[id = "value"]
    pub value: FloatParam,

    /// Persisted editor window state (size) so the host can restore it.
    #[persist = "editor-state"]
    editor_state: Arc<EguiState>,
}

impl Default for GradientKnobParams {
    fn default() -> Self {
        Self {
            value: FloatParam::new(
                "Value",
                50.0,
                FloatRange::Linear { min: 0.0, max: 100.0 },
            )
            .with_step_size(0.01)
            .with_unit(" %"),
            editor_state: EguiState::from_size(380, 580),
        }
    }
}

//==============================================================================
// Plugin
//==============================================================================
struct GradientKnobPlugin {
    params: Arc<GradientKnobParams>,
    // Smoothed/animated display value kept on the GUI side (ui.rs), the
    // processor itself is a pass-through.
}

impl Default for GradientKnobPlugin {
    fn default() -> Self {
        Self {
            params: Arc::new(GradientKnobParams::default()),
        }
    }
}

impl Plugin for GradientKnobPlugin {
    const NAME: &'static str = "Gradient Knob";
    const VENDOR: &'static str = "YourCompany";
    const URL: &'static str = "https://example.com";
    const EMAIL: &'static str = "info@example.com";
    const VERSION: &'static str = env!("CARGO_PKG_VERSION");

    const AUDIO_IO_LAYOUTS: &'static [AudioIOLayout] = &[AudioIOLayout {
        main_input_channels: NonZeroU32::new(2),
        main_output_channels: NonZeroU32::new(2),
        ..AudioIOLayout::const_default()
    }];

    const MIDI_INPUT: MidiConfig = MidiConfig::None;
    const MIDI_OUTPUT: MidiConfig = MidiConfig::None;
    const SAMPLE_ACCURATE_AUTOMATION: bool = true;

    type SysExMessage = ();
    type BackgroundTask = ();

    fn params(&self) -> Arc<dyn Params> {
        self.params.clone()
    }

    fn editor(&mut self, _async_executor: AsyncExecutor<Self>) -> Option<Box<dyn Editor>> {
        let params = self.params.clone();

        create_egui_editor(
            self.params.editor_state.clone(),
            GradientKnobUiState::default(),
            // build: one-time init (nothing needed here, matches eframe's
            // implicit Default::default() app construction)
            |_ctx, _state| {},
            // update: called every frame, this is the ported eframe::App::update body
            move |ctx, setter, ui_state| {
                ui::draw(ctx, setter, ui_state, &params);
            },
        )
    }

    fn initialize(
        &mut self,
        _audio_io_layout: &AudioIOLayout,
        _buffer_config: &BufferConfig,
        _context: &mut impl InitContext<Self>,
    ) -> bool {
        true
    }

    fn reset(&mut self) {}

    fn process(
        &mut self,
        _buffer: &mut Buffer,
        _aux: &mut AuxiliaryBuffers,
        _context: &mut impl ProcessContext<Self>,
    ) -> ProcessStatus {
        // Pass-through: this plugin's purpose is the animated knob UI, not DSP.
        ProcessStatus::Normal
    }
}

impl Vst3Plugin for GradientKnobPlugin {
    const VST3_CLASS_ID: [u8; 16] = *b"GradientKnobYCo!";
    const VST3_SUBCATEGORIES: &'static [Vst3SubCategory] =
        &[Vst3SubCategory::Fx, Vst3SubCategory::Tools];
}

nih_export_vst3!(GradientKnobPlugin);