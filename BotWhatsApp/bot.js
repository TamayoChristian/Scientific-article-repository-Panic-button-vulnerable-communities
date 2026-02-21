const mqtt = require('mqtt');
const { default: makeWASocket, useMultiFileAuthState, DisconnectReason } = require('@whiskeysockets/baileys');
const qrcode = require('qrcode-terminal');
const P = require('pino');
require('dotenv').config();

async function iniciarBot() {
    const { state, saveCreds } = await useMultiFileAuthState('./auth');

    const sock = makeWASocket({
        logger: P({ level: 'silent' }),
        printQRInTerminal: true,
        auth: state
    });

    sock.ev.on('creds.update', saveCreds);

    sock.ev.on('connection.update', (update) => {
        const { connection, lastDisconnect, qr } = update;
        if (qr) qrcode.generate(qr, { small: true });
        if (connection === 'close' && lastDisconnect?.error?.output?.statusCode !== DisconnectReason.loggedOut) iniciarBot();
    });

    // ===== Conexión MQTT TTN =====
    const mqttClient = mqtt.connect(process.env.MQTT_HOST, {
        username: process.env.MQTT_USER,
        password: process.env.MQTT_PASS
    });

    mqttClient.on('connect', () => {
        console.log('✅ Conectado a MQTT TTN');
        mqttClient.subscribe('#', (err) => { if (err) console.error(err); });
    });

    mqttClient.on('message', async (topic, msg) => {
        try {
            const payload = JSON.parse(msg.toString());
            if (!payload.uplink_message) return;

            const frmPayload = payload.uplink_message.frm_payload;
            if (!frmPayload) return;

            const decoded = Buffer.from(frmPayload, 'base64').toString('utf-8');
            let data = 'No hay datos';
            let latencia_texto = '';

            try {
                const jsonData = JSON.parse(decoded);

                if (jsonData.message === 'NO_FIX') {
                    data = `ID: ${jsonData.id || 'N/A'}\n⚠ GPS NO_FIX`;
                } else {
                    const id = jsonData.id || 'N/A';
                    const lat = jsonData.lat || 'N/A';
                    const lon = jsonData.lon || 'N/A';
                    const sats = jsonData.sats || 'N/A';
                    const ts = jsonData.ts || 'N/A';
                    data = `ID: ${id}\n📍 Lat: ${lat}\n📍 Lon: ${lon}\n🛰 Satellites: ${sats}`;

                    // Calcular latencia
                    if (ts && ts > 0) {
                        const ts_gps_ms = ts * 1000;
                        const received_ms = new Date(payload.received_at).getTime();
                        let latencia_ms = received_ms - ts_gps_ms;
                        if (latencia_ms < 0) latencia_ms = 20; // evitar negativa
                        latencia_texto = `\n⏳ Latency aprox: ${latencia_ms} ms`;
                    }
                }
            } catch (e) {
                data = decoded;
            }

            const fecha = new Date(payload.received_at);
            const hora = fecha.toLocaleTimeString('es-EC', { hour12: false });

            const texto_final = `¡EMERGENCY!
🕒 Time: ${hora}
📝 message: ${data}${latencia_texto}`;

            // Enviar mensaje a WhatsApp
            const destinos = ['593xxxxxxxxx@s.whatsapp.net'];
            for (const numero of destinos) {
                await sock.sendMessage(numero, { text: texto_final });
            }

        } catch (e) {
            console.error('❌ Error procesando mensaje MQTT:', e);
        }
    });
}

iniciarBot();
