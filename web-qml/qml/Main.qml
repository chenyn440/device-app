import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15
import "theme" as ThemeLib
import "components"

ApplicationWindow {
    id: root
    visible: true
    width: 1366
    height: 768
    title: "四极质谱上位机"
    color: ThemeLib.Theme.color.appBg

    property int currentPageIndex: 0
    property bool isWasm: Qt.platform.os === "wasm"
    property string uiFontFamily: cjkFont.status === FontLoader.Ready ? cjkFont.name : pickUIFontFamily()
    property int monitorSelectedRow: -1
    property int settingsConnectionTabIndex: 0
    property string settingsLocalHost: appState.connectionHost
    property int settingsLocalPort: appState.connectionPort
    property string settingsRemoteHost: "127.0.0.1"
    property int settingsRemotePort: 9001
    property bool settingsAutoSave: true
    property string settingsRunStatus: "运行状态    未连接"
    property int scanTabIndex: 0
    property bool scanFullMode: true
    property bool scanUseTargetVoltage: false
    property bool scanUseMassAxis: false
    property bool tuneShowAdvanced: false
    property int instrumentControlTabIndex: 0
    property int systemSettingsTabIndex: 0
    property int monitorDetectorMode: 0
    property int monitorStabilityMode: 0
    property int monitorScanMode: 0
    property bool chartFixedY: false
    property bool controlForePump: false
    property bool controlForeValve: false
    property bool controlMolecularPump: false
    property bool controlInletValve: false
    property bool controlFilament: false
    property bool controlMultiplier: false
    property bool tuneForePumpOn: false
    property bool tuneForeValveOn: false
    property bool tuneMolecularPumpOn: false
    property bool tuneInletValveOn: false
    property bool tuneFilamentOn: false
    property bool tuneMultiplierOn: false
    property double controlSourceTemp: 0.0
    property double controlChamberTemp: 0.0
    property int runConnectPortValue: appState.connectionPort
    property bool tuneAiBusy: false
    property string tuneAiStatus: "模型状态未知"
    property string tuneAiQuestion: ""
    property string tuneAiOutput: ""
    property bool tuneAiSummaryDialogActive: false
    property bool tuneAiAutoSummaryPreparing: false
    property int tuneAiAutoSummaryStartedAt: 0
    property int tuneAiAutoSummaryConnectAttempts: 0
    property int tuneAiAutoSummaryScanAttempts: 0
    property bool tuneAiAutoSummaryScanStarted: false
    property int tuneAiAutoSummaryTimeoutMs: 90000
    property real tuneAiAutoSummaryMassStart: 10
    property real tuneAiAutoSummaryMassEnd: 110
    property int tuneAiTick: 0
    property var tuneAiSessions: []
    property string tuneAiActiveSessionId: ""
    property string tuneAiChatInput: ""
    property bool tuneAiUseDeviceContext: false
    property string tuneAiProvider: "zhipu"
    property string tuneAiModel: "glm-4-flash"
    property string tuneAiBaseUrl: "https://open.bigmodel.cn/api/paas/v4"
    property string tuneAiApiKey: ""
    property var tuneAiSummaryHistoryItems: []
    property string tuneAiSummarySelectedId: ""
    property var tuneAiSummarySelectedEntry: ({})
    property string tuneAiSummaryLiveId: ""
    property string tuneAiSummaryLiveText: ""
    property bool tuneAiSummaryGenerating: false

    ListModel {
        id: simTableModel
        ListElement { enabled: true; mass: "18"; dwell: "80"; minv: "0"; maxv: "500" }
        ListElement { enabled: true; mass: "28"; dwell: "80"; minv: "0"; maxv: "500" }
        ListElement { enabled: true; mass: "44"; dwell: "80"; minv: "0"; maxv: "500" }
    }

    ListModel {
        id: monitorSimTableModel
        ListElement { enabled: true; mass: "79"; width: "1"; time: "5" }
        ListElement { enabled: true; mass: "94"; width: "1"; time: "5" }
        ListElement { enabled: true; mass: "109"; width: "1"; time: "5" }
        ListElement { enabled: true; mass: "124"; width: "1"; time: "5" }
        ListElement { enabled: false; mass: ""; width: ""; time: "" }
    }

    function f1(v) { return Number(v).toFixed(1) }
    function f2(v) { return Number(v).toFixed(2) }
    function pickUIFontFamily() {
        var candidates = [
            "PingFang SC",
            "Hiragino Sans GB",
            "Microsoft YaHei",
            "Noto Sans CJK SC",
            "Source Han Sans SC",
            "WenQuanYi Zen Hei"
        ]
        try {
            var families = Qt.fontFamilies()
            for (var i = 0; i < candidates.length; ++i) {
                if (families.indexOf(candidates[i]) >= 0) {
                    return candidates[i]
                }
            }
        } catch (e) {
        }
        return "sans-serif"
    }
    function requireConnected() {
        if (appState.connected) {
            return true
        }
        appState.toastRequested("设备未连接")
        return false
    }
    function apiRequest(method, path, payload, onSuccess, onError) {
        var xhr = new XMLHttpRequest()
        var requestUrl = path
        if (!root.isWasm && path.indexOf("http://") !== 0 && path.indexOf("https://") !== 0) {
            requestUrl = "http://127.0.0.1:8787" + path
        }
        xhr.open(method, requestUrl)
        xhr.setRequestHeader("Content-Type", "application/json")
        xhr.onreadystatechange = function() {
            if (xhr.readyState !== XMLHttpRequest.DONE) return
            var body = {}
            try { body = JSON.parse(xhr.responseText) } catch (e) {}
            if (xhr.status >= 200 && xhr.status < 300 && body.ok) {
                onSuccess(body.data)
            } else {
                var msg = ""
                if (body && body.error) msg = body.error
                if (!msg) msg = "HTTP " + xhr.status
                onError(msg)
            }
        }
        xhr.send(payload ? JSON.stringify(payload) : "{}")
    }
    function localAiRequest(kind, payload, onSuccess, onError) {
        var res = {}
        if (kind === "model/status") {
            res = appState.aiModelStatus()
        } else if (kind === "model/pull") {
            res = appState.aiPullModel((payload && payload.model) ? payload.model : "")
        } else if (kind === "tune/summary") {
            res = appState.aiTuneSummary()
        } else if (kind === "tune/troubleshoot") {
            res = appState.aiTuneTroubleshoot((payload && payload.question) ? payload.question : "")
        } else if (kind === "tune/export") {
            res = appState.aiTuneExport(
                        (payload && payload.question) ? payload.question : "",
                        !!(payload && payload.includeTroubleshoot))
        } else {
            onError("unsupported local ai request")
            return
        }
        if (res && res.ok) {
            onSuccess(res.data ? res.data : {})
            return
        }
        onError((res && res.error) ? res.error : "unknown error")
    }
    function handleNoFrameError(error, fallbackText) {
        if (error.indexOf("no frame available") >= 0) {
            tuneAiOutput = fallbackText
            appState.toastRequested("请先连接设备并采集一帧数据")
            return true
        }
        return false
    }
    function tuneAiRefreshModelStatus() {
        var onOk = function(data) {
            var provider = data.provider ? data.provider : (root.isWasm ? "gateway" : "local")
            tuneAiStatus = data.ready ? ("已就绪[" + provider + "]: " + data.model) : ("未安装[" + provider + "]: " + data.model)
            if (data && data.model) {
                root.tuneAiModel = data.model
            }
            if (data && data.provider) {
                root.tuneAiProvider = data.provider
            }
        }
        var onFail = function(error) {
            tuneAiStatus = "状态查询失败: " + error
            appState.toastRequested(tuneAiStatus)
        }
        if (root.isWasm) {
            apiRequest("GET", "/api/ai/model/status", null, onOk, onFail)
        } else {
            localAiRequest("model/status", {}, onOk, onFail)
        }
    }
    function tuneAiLoadProviderConfig() {
        if (root.isWasm) return
        var res = appState.aiProviderConfig()
        if (!(res && res.ok)) {
            return
        }
        var data = res.data ? res.data : {}
        root.tuneAiProvider = data.provider ? data.provider : "zhipu"
        root.tuneAiModel = data.model ? data.model : ""
        root.tuneAiBaseUrl = data.baseUrl ? data.baseUrl : ""
        root.tuneAiApiKey = data.apiKey ? data.apiKey : ""
    }
    function tuneAiProviderDefaults(provider) {
        var p = provider ? provider : "zhipu"
        if (p === "zhipu" || p === "cloud_zhipu") return { model: "glm-4-flash", baseUrl: "https://open.bigmodel.cn/api/paas/v4" }
        if (p === "gpt" || p === "openai" || p === "cloud_gpt") return { model: "gpt-4o-mini", baseUrl: "https://api.openai.com/v1" }
        if (p === "qwen" || p === "cloud_qwen") return { model: "qwen-max", baseUrl: "https://dashscope.aliyuncs.com/compatible-mode/v1" }
        if (p === "claude" || p === "cloud_claude") return { model: "claude-3-5-sonnet-latest", baseUrl: "https://api.anthropic.com/v1" }
        if (p === "local_ollama") return { model: "qwen3.5:2b", baseUrl: "http://127.0.0.1:11434" }
        return { model: "", baseUrl: "" }
    }
    function tuneAiApplyProviderDefaults() {
        var d = tuneAiProviderDefaults(root.tuneAiProvider)
        if (root.tuneAiModel.trim().length === 0) root.tuneAiModel = d.model
        if (root.tuneAiBaseUrl.trim().length === 0) root.tuneAiBaseUrl = d.baseUrl
    }
    function tuneAiSaveProviderConfig() {
        if (root.isWasm) {
            appState.toastRequested("Web 端暂不支持修改本地AI配置")
            return
        }
        var payload = {
            provider: root.tuneAiProvider,
            model: root.tuneAiModel,
            baseUrl: root.tuneAiBaseUrl,
            apiKey: root.tuneAiApiKey
        }
        var res = appState.aiSaveProviderConfig(payload)
        if (!(res && res.ok)) {
            appState.toastRequested("保存AI配置失败: " + ((res && res.error) ? res.error : "unknown"))
            return
        }
        var data = res.data ? res.data : {}
        root.tuneAiProvider = data.provider ? data.provider : root.tuneAiProvider
        root.tuneAiModel = data.model ? data.model : root.tuneAiModel
        root.tuneAiBaseUrl = data.baseUrl ? data.baseUrl : root.tuneAiBaseUrl
        root.tuneAiApiKey = data.apiKey ? data.apiKey : root.tuneAiApiKey
        appState.toastRequested("AI配置已保存并生效")
        tuneAiRefreshModelStatus()
    }
    function tuneAiPullModel() {
        if (tuneAiBusy) return
        tuneAiBusy = true
        var onOk = function(data) {
            tuneAiBusy = false
            tuneAiStatus = "已就绪: " + data.model
            appState.toastRequested("模型下载完成")
        }
        var onFail = function(error) {
            tuneAiBusy = false
            appState.toastRequested("模型下载失败: " + error)
        }
        if (root.isWasm) {
            apiRequest("POST", "/api/ai/model/pull", {}, onOk, onFail)
        } else {
            localAiRequest("model/pull", { model: "" }, onOk, onFail)
        }
    }
    function tuneAiSummary() {
        if (tuneAiBusy) return
        if (root.isWasm) {
            tuneAiBusy = true
            root.tuneAiSummaryGenerating = true
            root.tuneAiSummaryLiveId = "live-" + Date.now()
            root.tuneAiSummaryLiveText = ""
            root.tuneAiSummarySelectedId = root.tuneAiSummaryLiveId
            root.tuneAiSummarySelectedEntry = ({
                createdAt: new Date().toISOString(),
                model: (root.tuneAiModel && root.tuneAiModel.trim().length > 0) ? root.tuneAiModel : "未设置",
                content: ""
            })
            var onOk = function(data) {
                tuneAiBusy = false
                root.tuneAiSummaryGenerating = false
                tuneAiOutput = data.summary
                root.tuneAiSummaryLiveText = data.summary ? data.summary : ""
                tuneAiSummaryAppendHistory(root.tuneAiSummaryLiveText, false)
                tuneAiSummaryDialog.open()
            }
            var onFail = function(error) {
                tuneAiBusy = false
                root.tuneAiSummaryGenerating = false
                if (handleNoFrameError(error, "暂无可用于AI分析的谱图数据。\n请先连接设备并启动扫描，采集至少一帧后再点击 AI总结。")) return
                appState.toastRequested("AI总结失败: " + error)
            }
            apiRequest("POST", "/api/ai/tune/summary", {}, onOk, onFail)
        } else {
            if (!appState.connected || appState.spectrumPointCount <= 0) {
                tuneAiStartAutoPrepareSummary()
                return
            }
            tuneAiStartSummaryDesktop()
        }
    }
    function tuneAiOpenSummaryDialog() {
        if (!root.tuneAiSummaryGenerating) {
            var count = tuneAiSummaryLoadHistory()
            if (!tuneAiSummaryDialog.visible) {
                tuneAiSummaryDialog.open()
            }
            if (count <= 0) {
                tuneAiSummary()
            }
            return
        }
        if (!tuneAiSummaryDialog.visible) {
            tuneAiSummaryDialog.open()
        }
    }
    function tuneAiStartSummaryDesktop() {
        tuneAiAutoSummaryPreparing = false
        tuneAiAutoSummaryTimer.stop()
        tuneAiOutput = ""
        root.tuneAiSummaryLiveId = "live-" + Date.now()
        root.tuneAiSummaryLiveText = ""
        root.tuneAiSummaryGenerating = true
        root.tuneAiSummarySelectedId = root.tuneAiSummaryLiveId
        root.tuneAiSummarySelectedEntry = ({
            createdAt: new Date().toISOString(),
            model: (root.tuneAiModel && root.tuneAiModel.trim().length > 0) ? root.tuneAiModel : "未设置",
            content: ""
        })
        tuneAiBusy = true
        tuneAiSummaryDialogActive = true
        tuneAiSummaryDialog.open()
        if (!appState.aiTuneSummaryAsync()) {
            tuneAiSummaryDialogActive = false
            root.tuneAiSummaryGenerating = false
            tuneAiBusy = false
            appState.toastRequested("AI总结失败: 启动任务失败")
        }
    }
    function tuneAiStartAutoPrepareSummary() {
        if (root.isWasm || tuneAiBusy || tuneAiAutoSummaryPreparing) return
        tuneAiAutoSummaryPreparing = true
        tuneAiAutoSummaryStartedAt = Date.now()
        tuneAiAutoSummaryConnectAttempts = 0
        tuneAiAutoSummaryScanAttempts = 0
        tuneAiAutoSummaryScanStarted = false
        appState.toastRequested("正在自动连接设备并启动扫描...")
        tuneAiAutoSummaryTimer.start()
        tuneAiAutoPrepareSummaryStep()
    }
    function tuneAiAutoPrepareSummaryStep() {
        if (!tuneAiAutoSummaryPreparing || tuneAiBusy) return
        var elapsed = Date.now() - tuneAiAutoSummaryStartedAt
        if (elapsed > tuneAiAutoSummaryTimeoutMs) {
            tuneAiAutoSummaryPreparing = false
            tuneAiAutoSummaryTimer.stop()
            appState.toastRequested("自动连接/扫描超时，请检查设备状态后重试")
            return
        }
        if (!appState.connected) {
            if (tuneAiAutoSummaryConnectAttempts % 3 === 0) {
                var host = appState.connectionHost
                var port = appState.connectionPort
                if (tuneHostField && tuneHostField.text && tuneHostField.text.trim().length > 0) {
                    host = tuneHostField.text.trim()
                }
                if (tunePortField && tunePortField.value > 0) {
                    port = tunePortField.value
                }
                appState.connectToDevice(host, port)
            }
            tuneAiAutoSummaryConnectAttempts += 1
            return
        }
        if (!appState.scanning) {
            if (tuneAiAutoSummaryScanAttempts % 3 === 0) {
                tuneAiAutoSummaryScanStarted = true
                appState.startScan(tuneAiAutoSummaryMassStart, tuneAiAutoSummaryMassEnd)
                if (tuneAiAutoSummaryScanAttempts === 0) {
                    appState.toastRequested("设备已连接，正在采集扫描数据...")
                }
            }
            tuneAiAutoSummaryScanAttempts += 1
            return
        }
        if (appState.spectrumPointCount > 0) {
            appState.toastRequested("已采集到谱图，开始AI总结")
            tuneAiStartSummaryDesktop()
        }
    }
    function tuneAiTroubleshoot() {
        if (tuneAiBusy) return
        if (tuneAiQuestion.trim().length === 0) {
            appState.toastRequested("请先输入疑难问题")
            return
        }
        var payload = { question: tuneAiQuestion }
        if (root.isWasm) {
            tuneAiBusy = true
            var onOk = function(data) {
                tuneAiBusy = false
                tuneAiOutput = data.answer
            }
            var onFail = function(error) {
                tuneAiBusy = false
                if (handleNoFrameError(error, "暂无可用于AI分析的谱图数据。\n请先连接设备并启动扫描，采集至少一帧后再进行疑难解答。")) return
                appState.toastRequested("疑难解答失败: " + error)
            }
            apiRequest("POST", "/api/ai/tune/troubleshoot", payload, onOk, onFail)
        } else {
            tuneAiOutput = ""
            tuneAiBusy = true
            if (!appState.aiTuneTroubleshootAsync(tuneAiQuestion)) {
                tuneAiBusy = false
                appState.toastRequested("疑难解答失败: 启动任务失败")
            }
        }
    }
    function tuneAiExport() {
        if (tuneAiBusy) return
        var payload = { question: tuneAiQuestion, includeTroubleshoot: tuneAiQuestion.trim().length > 0 }
        if (root.isWasm) {
            tuneAiBusy = true
            var onOk = function(data) {
                tuneAiBusy = false
                tuneAiOutput = data.summary + (data.troubleshoot ? "\n\n" + data.troubleshoot : "")
                appState.toastRequested("导出成功: " + data.paths.reportTxt)
            }
            var onFail = function(error) {
                tuneAiBusy = false
                if (handleNoFrameError(error, "暂无可导出的谱图数据。\n请先连接设备并启动扫描，采集至少一帧后再导出AI报告。")) return
                appState.toastRequested("导出失败: " + error)
            }
            apiRequest("POST", "/api/ai/tune/export", payload, onOk, onFail)
        } else {
            tuneAiOutput = ""
            tuneAiBusy = true
            if (!appState.aiTuneExportAsync(tuneAiQuestion, tuneAiQuestion.trim().length > 0)) {
                tuneAiBusy = false
                appState.toastRequested("导出失败: 启动任务失败")
            }
        }
    }
    function tuneAiCancel() {
        tuneAiAutoSummaryPreparing = false
        tuneAiAutoSummaryTimer.stop()
        if (!tuneAiBusy) return
        if (root.isWasm) {
            appState.toastRequested("Web 端暂不支持取消")
            return
        }
        appState.aiCancel()
    }
    function tuneAiBusyText() {
        var dots = ""
        var count = (tuneAiTick % 4)
        for (var i = 0; i < count; ++i) dots += "."
        return "模型正在思考" + dots
    }
    function tuneAiSummaryFormatTime(isoText) {
        var t = isoText ? isoText : ""
        if (t.length === 0) return ""
        try {
            var d = new Date(t)
            if (isNaN(d.getTime())) return t
            var mm = (d.getMonth() + 1).toString().padStart(2, "0")
            var dd = d.getDate().toString().padStart(2, "0")
            var hh = d.getHours().toString().padStart(2, "0")
            var mi = d.getMinutes().toString().padStart(2, "0")
            var ss = d.getSeconds().toString().padStart(2, "0")
            return d.getFullYear() + "-" + mm + "-" + dd + " " + hh + ":" + mi + ":" + ss
        } catch (e) {
            return t
        }
    }
    function tuneAiSummaryApplyHistory(items, preferredId) {
        var list = items ? items.slice(0) : []
        root.tuneAiSummaryHistoryItems = list
        var targetId = preferredId && preferredId.length > 0 ? preferredId : root.tuneAiSummarySelectedId
        var picked = null
        for (var i = 0; i < list.length; ++i) {
            var it = list[i]
            if (targetId.length > 0 && it.id === targetId) {
                picked = it
                break
            }
        }
        if (!picked && list.length > 0) picked = list[0]
        if (!picked) {
            root.tuneAiSummarySelectedId = ""
            root.tuneAiSummarySelectedEntry = ({})
            return
        }
        root.tuneAiSummarySelectedId = picked.id ? picked.id : ""
        root.tuneAiSummarySelectedEntry = picked
    }
    function tuneAiSummaryLoadHistory() {
        if (root.isWasm) return root.tuneAiSummaryHistoryItems.length
        var res = appState.aiSummaryHistory()
        if (!(res && res.ok)) return root.tuneAiSummaryHistoryItems.length
        var data = res.data ? res.data : {}
        var items = data.items ? data.items : []
        tuneAiSummaryApplyHistory(items, "")
        return items.length
    }
    function tuneAiSummaryAppendHistory(summaryText, keepCurrentSelection) {
        var content = summaryText ? summaryText.trim() : ""
        if (content.length === 0) return
        var item = {
            createdAt: new Date().toISOString(),
            model: (root.tuneAiModel && root.tuneAiModel.trim().length > 0) ? root.tuneAiModel : "未设置",
            content: content
        }
        var keepSelection = keepCurrentSelection === true
        var preferredId = keepSelection ? root.tuneAiSummarySelectedId : item.id
        if (root.isWasm) {
            item.id = "local-" + Date.now()
            var localItems = root.tuneAiSummaryHistoryItems ? root.tuneAiSummaryHistoryItems.slice(0) : []
            localItems.unshift(item)
            if (localItems.length > 50) localItems = localItems.slice(0, 50)
            preferredId = keepSelection ? root.tuneAiSummarySelectedId : item.id
            tuneAiSummaryApplyHistory(localItems, preferredId)
            return
        }
        var res = appState.aiSummaryAppend(item)
        if (!(res && res.ok)) {
            appState.toastRequested("保存AI总结历史失败")
            return
        }
        var data = res.data ? res.data : {}
        var saved = data.item ? data.item : item
        preferredId = keepSelection ? root.tuneAiSummarySelectedId : (saved.id ? saved.id : "")
        tuneAiSummaryApplyHistory(data.items ? data.items : [], preferredId)
    }
    function tuneAiSummarySelectHistory(item) {
        if (!item) return
        root.tuneAiSummarySelectedId = item.id ? item.id : ""
        root.tuneAiSummarySelectedEntry = item
    }
    function tuneAiSummaryDisplayedText() {
        if (root.tuneAiSummaryLiveId.length > 0 && root.tuneAiSummarySelectedId === root.tuneAiSummaryLiveId) {
            return root.tuneAiSummaryLiveText
        }
        if (root.tuneAiSummarySelectedEntry && root.tuneAiSummarySelectedEntry.content) {
            return root.tuneAiSummarySelectedEntry.content
        }
        return ""
    }
    function tuneAiSummaryDisplayedModel() {
        if (root.tuneAiSummaryLiveId.length > 0 && root.tuneAiSummarySelectedId === root.tuneAiSummaryLiveId) {
            return (root.tuneAiModel && root.tuneAiModel.trim().length > 0) ? root.tuneAiModel : "未设置"
        }
        if (root.tuneAiSummarySelectedEntry && root.tuneAiSummarySelectedEntry.model) {
            return root.tuneAiSummarySelectedEntry.model
        }
        return (root.tuneAiModel && root.tuneAiModel.trim().length > 0) ? root.tuneAiModel : "未设置"
    }
    function tuneAiSummaryDisplayedCreatedAt() {
        if (root.tuneAiSummaryLiveId.length > 0 && root.tuneAiSummarySelectedId === root.tuneAiSummaryLiveId) {
            if (root.tuneAiSummarySelectedEntry && root.tuneAiSummarySelectedEntry.createdAt) {
                return root.tuneAiSummarySelectedEntry.createdAt
            }
            return ""
        }
        if (root.tuneAiSummarySelectedEntry && root.tuneAiSummarySelectedEntry.createdAt) {
            return root.tuneAiSummarySelectedEntry.createdAt
        }
        return ""
    }
    function tuneAiSummaryExportSelected() {
        var item = root.tuneAiSummarySelectedEntry ? root.tuneAiSummarySelectedEntry : ({})
        var content = item.content ? item.content : tuneAiSummaryDisplayedText()
        if (!content || content.trim().length === 0) {
            appState.toastRequested("暂无可导出的AI总结")
            return
        }
        if (root.isWasm) {
            appState.toastRequested("Web 端暂不支持弹窗历史导出")
            return
        }
        var model = item.model ? item.model : root.tuneAiModel
        var createdAt = item.createdAt ? item.createdAt : ""
        var res = appState.aiSummaryExportText(content, model, createdAt)
        if (!(res && res.ok)) {
            appState.toastRequested("导出失败: " + ((res && res.error) ? res.error : "unknown"))
            return
        }
        var data = res.data ? res.data : {}
        appState.toastRequested("导出成功: " + (data.reportTxt ? data.reportTxt : ""))
    }
    function tuneAiApplySnapshot(data) {
        if (!data) return
        root.tuneAiSessions = data.sessions ? data.sessions : []
        root.tuneAiActiveSessionId = data.activeSessionId ? data.activeSessionId : ""
    }
    function tuneAiFindSessionIndex(sessionId) {
        for (var i = 0; i < root.tuneAiSessions.length; ++i) {
            if (root.tuneAiSessions[i].id === sessionId) return i
        }
        return -1
    }
    function tuneAiActiveSessionIndex() {
        var idx = tuneAiFindSessionIndex(root.tuneAiActiveSessionId)
        return idx < 0 ? 0 : idx
    }
    function tuneAiActiveMessages() {
        var idx = tuneAiFindSessionIndex(root.tuneAiActiveSessionId)
        if (idx < 0) return []
        var s = root.tuneAiSessions[idx]
        if (!s.messages) return []
        var out = []
        for (var i = 0; i < s.messages.length; ++i) {
            var m = s.messages[i]
            var c = m && m.content ? m.content : ""
            if (c.trim().length === 0) continue
            out.push(m)
        }
        return out
    }
    function tuneAiChatBootstrap() {
        if (root.isWasm) return
        var res = appState.aiChatBootstrap()
        if (res && res.ok) {
            tuneAiApplySnapshot(res.data ? res.data : {})
        }
    }
    function tuneAiNewSession() {
        if (root.isWasm || tuneAiBusy) return
        var res = appState.aiChatNewSession()
        if (res && res.ok) {
            tuneAiApplySnapshot(res.data ? res.data : {})
            root.tuneAiChatInput = ""
            return
        }
        appState.toastRequested("新建会话失败")
    }
    function tuneAiSwitchSession(sessionId) {
        if (root.isWasm || tuneAiBusy) return
        var id = sessionId ? sessionId : ""
        if (id.length === 0) return
        var res = appState.aiChatSwitchSession(id)
        if (res && res.ok) {
            tuneAiApplySnapshot(res.data ? res.data : {})
        }
    }
    function tuneAiClearCurrentSession() {
        if (root.isWasm || tuneAiBusy) return
        var res = appState.aiChatClearCurrentSession()
        if (res && res.ok) {
            tuneAiApplySnapshot(res.data ? res.data : {})
            return
        }
        appState.toastRequested("清空会话失败")
    }
    function tuneAiSendChat() {
        if (root.isWasm) {
            appState.toastRequested("Web 端暂不支持多轮聊天")
            return
        }
        if (tuneAiBusy) return
        var question = root.tuneAiChatInput.trim()
        if (question.length === 0) {
            appState.toastRequested("请输入问题")
            return
        }
        var res = appState.aiChatSendAsync(question, root.tuneAiUseDeviceContext)
        if (!(res && res.ok)) {
            appState.toastRequested("发送失败: " + ((res && res.error) ? res.error : "unknown"))
            return
        }
        tuneAiApplySnapshot(res.data ? res.data : {})
        root.tuneAiChatInput = ""
        if (root.tuneAiUseDeviceContext && !(res.data && res.data.deviceContextAttached)) {
            appState.toastRequested("当前无设备数据，已按通用模式回答")
        }
    }
    function tuneAiApplyChatChunk(sessionId, messageId, chunk) {
        var sIdx = tuneAiFindSessionIndex(sessionId)
        if (sIdx < 0) return
        var sessions = root.tuneAiSessions
        var session = sessions[sIdx]
        if (!session.messages) return
        for (var i = 0; i < session.messages.length; ++i) {
            if (session.messages[i].id === messageId) {
                var m = session.messages[i]
                m.content = (m.content ? m.content : "") + chunk
                session.messages[i] = m
                sessions[sIdx] = session
                root.tuneAiSessions = sessions.slice(0)
                return
            }
        }
    }
    FontLoader {
        id: cjkFont
        source: "qrc:/qml/fonts/HiraginoSansGB.ttc"
    }

    font.family: uiFontFamily

    Component.onCompleted: {
        tuneAiLoadProviderConfig()
        tuneAiApplyProviderDefaults()
        tuneAiRefreshModelStatus()
        tuneAiChatBootstrap()
        tuneAiSummaryLoadHistory()
    }

    Timer {
        interval: 450
        repeat: true
        running: tuneAiBusy
        onTriggered: tuneAiTick = (tuneAiTick + 1) % 1000
    }

    Timer {
        id: tuneAiAutoSummaryTimer
        interval: 800
        repeat: true
        running: tuneAiAutoSummaryPreparing
        onTriggered: tuneAiAutoPrepareSummaryStep()
    }

    Component {
        id: webLineChartComponent
        Rectangle {
            id: webChart
            property string chartTitle: ""
            property string chartKey: "spectrum"
            property bool autoX: false
            property bool autoY: true
            property real fixedXMin: 0
            property real fixedXMax: 100
            property real fixedYMin: 0
            property real fixedYMax: 1000
            property var pointsData: []
            color: "#f8fafc"
            border.color: "#d8dee8"

            function refresh() {
                pointsData = appState.webChartPoints(chartKey)
                chartCanvas.requestPaint()
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    text: webChart.chartTitle
                    font.pixelSize: 12
                    font.bold: true
                    color: "#344054"
                }

                Canvas {
                    id: chartCanvas
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    onPaint: {
                        try {
                            var ctx = getContext("2d")
                            if (ctx.reset) {
                                ctx.reset()
                            } else {
                                ctx.setTransform(1, 0, 0, 1, 0, 0)
                            }
                            ctx.fillStyle = "#f8fafc"
                            ctx.fillRect(0, 0, width, height)

                            var pts = webChart.pointsData
                            var left = 6
                            var top = 4
                            var right = width - 6
                            var bottom = height - 4
                            var w = Math.max(1, right - left)
                            var h = Math.max(1, bottom - top)

                            ctx.strokeStyle = "#e7ecf3"
                            ctx.lineWidth = 1
                            for (var i = 0; i <= 4; ++i) {
                                var gy = top + h * i / 4.0
                                ctx.beginPath()
                                ctx.moveTo(left, gy)
                                ctx.lineTo(right, gy)
                                ctx.stroke()
                            }

                            if (!pts || pts.length < 2) {
                                return
                            }

                            var xMin = webChart.fixedXMin
                            var xMax = webChart.fixedXMax
                            var yMin = webChart.fixedYMin
                            var yMax = webChart.fixedYMax

                            if (webChart.autoX) {
                                xMin = pts[0].x
                                xMax = pts[0].x
                                for (var ix = 1; ix < pts.length; ++ix) {
                                    xMin = Math.min(xMin, pts[ix].x)
                                    xMax = Math.max(xMax, pts[ix].x)
                                }
                            }
                            if (webChart.autoY) {
                                yMin = pts[0].y
                                yMax = pts[0].y
                                for (var iy = 1; iy < pts.length; ++iy) {
                                    yMin = Math.min(yMin, pts[iy].y)
                                    yMax = Math.max(yMax, pts[iy].y)
                                }
                                yMin = Math.min(0, yMin)
                                yMax = yMax * 1.08
                            }
                            if (xMax <= xMin) {
                                xMax = xMin + 1
                            }
                            if (yMax <= yMin) {
                                yMax = yMin + 1
                            }

                            ctx.strokeStyle = ThemeLib.Theme.color.linePrimary
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            for (var p = 0; p < pts.length; ++p) {
                                var px = left + (pts[p].x - xMin) * w / (xMax - xMin)
                                var py = bottom - (pts[p].y - yMin) * h / (yMax - yMin)
                                if (p === 0) {
                                    ctx.moveTo(px, py)
                                } else {
                                    ctx.lineTo(px, py)
                                }
                            }
                            ctx.stroke()
                        } catch (e) {
                            // Keep UI responsive even if canvas backend throws in wasm.
                        }
                    }
                }
            }

            Connections {
                target: appState
                function onFrameChanged() {
                    webChart.refresh()
                }
            }

            Component.onCompleted: refresh()
        }
    }

    Component {
        id: appMenuBarItemDelegate
        MenuBarItem {
            id: barItem
            implicitWidth: 88
            implicitHeight: 46
            font.pixelSize: ThemeLib.Theme.font.menu
            font.family: root.uiFontFamily

            contentItem: Text {
                text: barItem.text
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: "#2f3a4a"
                font: barItem.font
            }

            background: Rectangle {
                color: barItem.highlighted || barItem.down ? "#b5b8bd" : "transparent"
                border.color: "transparent"
            }
        }
    }

    Component {
        id: appMenuItemDelegate
        MenuItem {
            id: menuItem
            implicitWidth: 220
            implicitHeight: 56
            font.pixelSize: 22
            font.family: root.uiFontFamily

            contentItem: Text {
                text: menuItem.text
                leftPadding: 18
                rightPadding: 18
                verticalAlignment: Text.AlignVCenter
                color: "#2f3a4a"
                font: menuItem.font
            }

            background: Rectangle {
                color: menuItem.highlighted ? "#eceff3" : "transparent"
                border.color: "transparent"
            }
        }
    }

    menuBar: MenuBar {
        implicitHeight: ThemeLib.Theme.font.menu + 18
        font.pixelSize: ThemeLib.Theme.font.menu
        Menu {
            title: "文件"
            MenuItem { text: "打开单帧数据"; onTriggered: root.currentPageIndex = 3 }
            MenuItem { text: "退出"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "运行"
            MenuItem {
                text: "仪器连接"
                onTriggered: {
                    runConnectHost.text = appState.connectionHost
                    runConnectPortValue = appState.connectionPort
                    runConnectionDialog.open()
                }
            }
            MenuItem { text: "断开连接"; onTriggered: appState.disconnectFromDevice() }
            MenuItem { text: "仪器控制"; onTriggered: instrumentControlDialog.open() }
        }
        Menu {
            title: "设置"
            MenuItem { text: "扫描设置"; onTriggered: scanSettingsDialog.open() }
            MenuItem { text: "数据处理"; onTriggered: dataProcessingDialog.open() }
            MenuItem { text: "系统设置"; onTriggered: systemSettingsDialog.open() }
            MenuItem { text: "谱图设置"; onTriggered: chartSettingsDialog.open() }
        }
        Menu {
            title: "视图"
            MenuItem { text: "切换到调谐"; onTriggered: root.currentPageIndex = 0 }
            MenuItem { text: "切换到监测"; onTriggered: root.currentPageIndex = 1 }
            MenuItem { text: "切换到方法"; onTriggered: root.currentPageIndex = 2 }
            MenuItem { text: "切换到数据"; onTriggered: root.currentPageIndex = 3 }
            MenuItem { text: "切换到设置"; onTriggered: root.currentPageIndex = 4 }
        }
    }

    Connections {
        target: appState
        function onToastRequested(message) {
            toast.messageText = message
            toast.open()
        }
        function onStatusChanged() {
            if (appState.connected) {
                settingsRunStatus = "运行状态    已连接"
            } else if (settingsRunStatus !== "运行状态    存储设置已保存") {
                settingsRunStatus = "运行状态    未连接"
            }
            tuneForePumpOn = appState.switchState("forePump")
            tuneForeValveOn = appState.switchState("foreValve")
            tuneMolecularPumpOn = appState.switchState("molecularPump")
            tuneInletValveOn = appState.switchState("inletValve")
            tuneFilamentOn = appState.switchState("filament")
            tuneMultiplierOn = appState.switchState("multiplier")
        }
        function onAiRunningChanged() {
            if (!root.isWasm) {
                tuneAiBusy = appState.aiRunning
            }
        }
        function onAiStreamChunk(chunk) {
            if (!root.isWasm) {
                root.tuneAiOutput += chunk
                if (root.tuneAiSummaryDialogActive) {
                    root.tuneAiSummaryLiveText += chunk
                }
            }
        }
        function onAiRequestFinished(success, message, data) {
            if (root.isWasm) {
                return
            }
            tuneAiAutoSummaryPreparing = false
            tuneAiAutoSummaryTimer.stop()
            tuneAiBusy = false
            if (success) {
                if (data && data.type === "summary") {
                    root.tuneAiSummaryDialogActive = false
                    root.tuneAiSummaryGenerating = false
                    root.tuneAiSummaryLiveText = data.summary ? data.summary : root.tuneAiSummaryLiveText
                    tuneAiSummaryAppendHistory(root.tuneAiSummaryLiveText, true)
                    if (!tuneAiSummaryDialog.visible) {
                        tuneAiSummaryDialog.open()
                    }
                }
                if (data && data.type === "export") {
                    appState.toastRequested("导出成功: " + data.paths.reportTxt)
                }
                return
            }
            root.tuneAiSummaryDialogActive = false
            root.tuneAiSummaryGenerating = false
            if (handleNoFrameError(message, "暂无可用于AI分析的谱图数据。\n请先连接设备并启动扫描，采集至少一帧后再点击 AI总结。")) {
                return
            }
            if (message === "已取消生成") {
                root.tuneAiOutput += "\n\n[已取消生成]"
                if (root.tuneAiSummaryLiveText.length > 0) {
                    root.tuneAiSummaryLiveText += "\n\n[已取消生成]"
                } else {
                    root.tuneAiSummaryLiveText = "[已取消生成]"
                }
                return
            }
            appState.toastRequested("AI任务失败: " + message)
        }
        function onAiChatStreamChunk(sessionId, messageId, chunk) {
            if (root.isWasm) {
                return
            }
            tuneAiApplyChatChunk(sessionId, messageId, chunk)
        }
        function onAiChatFinished(success, message, data) {
            if (root.isWasm) {
                return
            }
            if (data) {
                tuneAiApplySnapshot(data)
            }
            if (!success && message) {
                appState.toastRequested("聊天失败: " + message)
            }
        }
    }

    Dialog {
        id: toast
        modal: false
        standardButtons: Dialog.Ok
        property string messageText: ""
        contentItem: Label {
            text: toast.messageText
            wrapMode: Text.WordWrap
            padding: 10
        }
        x: root.width - width - 20
        y: 16
    }

    Dialog {
        id: tuneAiSummaryDialog
        title: ""
        modal: true
        focus: true
        width: Math.min(1040, Math.max(680, root.width * 0.82))
        height: Math.min(620, Math.max(360, root.height * 0.72))
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onOpened: if (!root.tuneAiSummaryGenerating) tuneAiSummaryLoadHistory()
        header: Rectangle {
            implicitHeight: 40
            color: "#f9fafb"
            border.color: "#d0d5dd"
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8
                Label {
                    text: "AI总结"
                    font.pixelSize: 13
                    color: "#1f2937"
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                AppButton {
                    text: "模型配置"
                    implicitWidth: 72
                    enabled: !tuneAiBusy
                    onClicked: tuneAiProviderDialog.open()
                }
                AppButton {
                    text: "发起AI总结"
                    implicitWidth: 84
                    visible: root.tuneAiSummaryHistoryItems.length > 0
                    enabled: !tuneAiBusy && !root.tuneAiSummaryGenerating
                    onClicked: tuneAiSummary()
                }
            }
        }

        contentItem: Rectangle {
            color: "#f9fafb"
            border.color: "#d0d5dd"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Label {
                    Layout.fillWidth: true
                    text: "模型: " + tuneAiSummaryDisplayedModel()
                    color: "#475467"
                    font.pixelSize: 12
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 12

                    Item {
                        Layout.preferredWidth: 270
                        Layout.fillHeight: true
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            anchors.topMargin: 3
                            color: "#16000000"
                            radius: 6
                        }
                        Rectangle {
                            anchors.fill: parent
                            color: "#ffffff"
                            border.color: "#e5e7eb"
                            radius: 6
                            clip: true

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 8

                                Label {
                                    Layout.fillWidth: true
                                    text: "AI总结历史"
                                    color: "#344054"
                                    font.pixelSize: 12
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "#f8fafc"
                                    radius: 4
                                    clip: true

                                    ListView {
                                        id: tuneAiSummaryHistoryList
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        spacing: 6
                                        model: root.tuneAiSummaryHistoryItems
                                        delegate: Rectangle {
                                            width: tuneAiSummaryHistoryList.width
                                            radius: 4
                                            color: (modelData.id && root.tuneAiSummarySelectedId === modelData.id) ? "#dbeafe" : "#ffffff"
                                            border.width: (modelData.id && root.tuneAiSummarySelectedId === modelData.id) ? 1 : 0
                                            border.color: "#93c5fd"
                                            implicitHeight: 44
                                            Column {
                                                anchors.fill: parent
                                                anchors.margins: 6
                                                spacing: 2
                                                Text {
                                                    text: tuneAiSummaryFormatTime(modelData.createdAt ? modelData.createdAt : "")
                                                    font.pixelSize: 11
                                                    color: "#475467"
                                                    elide: Text.ElideRight
                                                    width: parent.width
                                                }
                                                Text {
                                                    text: modelData.model ? modelData.model : "未设置"
                                                    font.pixelSize: 11
                                                    color: "#667085"
                                                    elide: Text.ElideRight
                                                    width: parent.width
                                                }
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: tuneAiSummarySelectHistory(modelData)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            anchors.topMargin: 3
                            color: "#16000000"
                            radius: 6
                        }
                        Rectangle {
                            anchors.fill: parent
                            color: "#ffffff"
                            border.color: "#e5e7eb"
                            radius: 6
                            clip: true

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 8

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        Layout.fillWidth: true
                                        text: (root.tuneAiSummaryLiveId.length > 0 && root.tuneAiSummarySelectedId === root.tuneAiSummaryLiveId)
                                              ? "条目: 当前生成"
                                              : (tuneAiSummaryDisplayedCreatedAt().length > 0
                                                 ? ("条目: " + tuneAiSummaryFormatTime(tuneAiSummaryDisplayedCreatedAt()))
                                                 : "条目: 当前结果")
                                        color: "#667085"
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                    AppButton {
                                        text: "导出AI报告"
                                        implicitWidth: 92
                                        enabled: !root.tuneAiSummaryGenerating && tuneAiSummaryDisplayedText().trim().length > 0
                                        onClicked: tuneAiSummaryExportSelected()
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    visible: root.tuneAiSummaryGenerating
                                             && root.tuneAiSummaryLiveId.length > 0
                                             && root.tuneAiSummarySelectedId === root.tuneAiSummaryLiveId
                                    text: tuneAiBusyText()
                                    color: "#2563eb"
                                    font.pixelSize: 12
                                }

                                ScrollView {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    clip: true
                                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                                    TextArea {
                                        width: parent.width
                                        readOnly: true
                                        wrapMode: TextEdit.Wrap
                                        text: tuneAiSummaryDisplayedText()
                                        placeholderText: "AI总结结果"
                                        font.pixelSize: 12
                                        selectByMouse: true
                                        onTextChanged: cursorPosition = length
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    AppButton {
                        text: "关闭"
                        implicitWidth: 72
                        onClicked: tuneAiSummaryDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: tuneAiProviderDialog
        title: "模型配置"
        modal: true
        focus: true
        width: 760
        height: 350
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton

        property string draftProvider: "zhipu"
        property string draftModel: ""
        property string draftBaseUrl: ""
        property string draftApiKey: ""

        onOpened: {
            draftProvider = root.tuneAiProvider
            draftModel = root.tuneAiModel
            draftBaseUrl = root.tuneAiBaseUrl
            draftApiKey = root.tuneAiApiKey
            if (draftModel.trim().length === 0 || draftBaseUrl.trim().length === 0) {
                var d = tuneAiProviderDefaults(draftProvider)
                if (draftModel.trim().length === 0) draftModel = d.model
                if (draftBaseUrl.trim().length === 0) draftBaseUrl = d.baseUrl
            }
        }

        contentItem: Rectangle {
            color: "#f9fafb"
            border.color: "#d0d5dd"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ffffff"
                    border.color: "#d0d5dd"

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        columns: 2
                        rowSpacing: 10
                        columnSpacing: 10

                        Label { text: "Provider"; color: "#475467"; font.pixelSize: 12; Layout.preferredWidth: 64 }
                        ComboBox {
                            Layout.fillWidth: true
                            implicitHeight: 30
                            model: ["zhipu", "gpt", "qwen", "claude"]
                            currentIndex: Math.max(0, model.indexOf(tuneAiProviderDialog.draftProvider))
                            enabled: !tuneAiBusy
                            font.pixelSize: 12
                            onActivated: {
                                tuneAiProviderDialog.draftProvider = model[index]
                                var d = tuneAiProviderDefaults(tuneAiProviderDialog.draftProvider)
                                tuneAiProviderDialog.draftModel = d.model
                                tuneAiProviderDialog.draftBaseUrl = d.baseUrl
                            }
                        }

                        Label { text: "模型"; color: "#475467"; font.pixelSize: 12; Layout.preferredWidth: 64 }
                        TextField {
                            Layout.fillWidth: true
                            implicitHeight: 30
                            text: tuneAiProviderDialog.draftModel
                            placeholderText: "例如 glm-4-flash / gpt-4o-mini / qwen-max / claude-3-5-sonnet-latest"
                            enabled: !tuneAiBusy
                            padding: 8
                            font.pixelSize: 12
                            onTextEdited: tuneAiProviderDialog.draftModel = text
                        }

                        Label { text: "BaseURL"; color: "#475467"; font.pixelSize: 12; Layout.preferredWidth: 64 }
                        TextField {
                            Layout.fillWidth: true
                            implicitHeight: 30
                            text: tuneAiProviderDialog.draftBaseUrl
                            placeholderText: "API Base URL"
                            enabled: !tuneAiBusy
                            padding: 8
                            font.pixelSize: 12
                            onTextEdited: tuneAiProviderDialog.draftBaseUrl = text
                        }

                        Label { text: "API Key"; color: "#475467"; font.pixelSize: 12; Layout.preferredWidth: 64 }
                        TextField {
                            Layout.fillWidth: true
                            implicitHeight: 30
                            text: tuneAiProviderDialog.draftApiKey
                            echoMode: TextInput.Password
                            placeholderText: "输入云端 API Key"
                            enabled: !tuneAiBusy
                            padding: 8
                            font.pixelSize: 12
                            onTextEdited: tuneAiProviderDialog.draftApiKey = text
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    AppButton {
                        text: "取消"
                        implicitWidth: 72
                        enabled: !tuneAiBusy
                        onClicked: tuneAiProviderDialog.close()
                    }
                    AppButton {
                        text: "保存并生效"
                        implicitWidth: 96
                        enabled: !tuneAiBusy
                        onClicked: {
                            root.tuneAiProvider = tuneAiProviderDialog.draftProvider
                            root.tuneAiModel = tuneAiProviderDialog.draftModel
                            root.tuneAiBaseUrl = tuneAiProviderDialog.draftBaseUrl
                            root.tuneAiApiKey = tuneAiProviderDialog.draftApiKey
                            tuneAiSaveProviderConfig()
                            tuneAiProviderDialog.close()
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        z: 999
        visible: false
        color: "transparent"
        MouseArea {
            anchors.fill: parent
            onClicked: root.topMenuVisible = false
        }
    }

    Rectangle {
        x: 0
        y: 0
        z: 1000
        width: 230
        height: 0
        visible: false
        color: "#ffffff"
        border.color: "#a9adb7"
        border.width: 1

        Column {
            id: topMenuColumn
            anchors.fill: parent
            Repeater {
                model: 0
                delegate: Rectangle {
                    width: 230
                    height: 48
                    color: menuHover.containsMouse ? "#eceff3" : "#ffffff"
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        text: ""
                        color: "#2f3a4a"
                        font.pixelSize: 16
                        font.family: root.uiFontFamily
                    }
                    MouseArea {
                        id: menuHover
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            // disabled
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: runConnectionDialog
        title: "仪器连接"
        modal: true
        width: 408
        height: 256
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#f9fafb"
                    border.color: "#d0d5dd"

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        columns: 2
                        rowSpacing: 10
                        columnSpacing: 10

                        Label { text: "IP"; Layout.preferredWidth: 30 }
                        TextField {
                            id: runConnectHost
                            Layout.fillWidth: true
                            implicitHeight: 30
                            text: "127.0.0.1"
                            padding: 8
                        }

                        Label { text: "端口"; Layout.preferredWidth: 30 }
                        Rectangle {
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30
                            color: "#ffffff"
                            border.color: "#cfd4dc"

                            RowLayout {
                                anchors.fill: parent
                                spacing: 0
                                Rectangle {
                                    Layout.preferredWidth: 38
                                    Layout.fillHeight: true
                                    color: "#f1f3f5"
                                    border.color: "#d1d5db"
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 20; color: "#475467" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: runConnectPortValue = Math.max(1, runConnectPortValue - 1)
                                    }
                                }
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "#ffffff"
                                    Text {
                                        anchors.centerIn: parent
                                        text: Number(runConnectPortValue).toLocaleString(Qt.locale("en_US"), 'f', 0)
                                        font.pixelSize: 12
                                        color: "#344054"
                                    }
                                }
                                Rectangle {
                                    Layout.preferredWidth: 38
                                    Layout.fillHeight: true
                                    color: "#f1f3f5"
                                    border.color: "#d1d5db"
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 24; color: "#101828" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: runConnectPortValue = Math.min(65535, runConnectPortValue + 1)
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    color: "transparent"
                    border.color: "transparent"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "取消"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: runConnectionDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#1d64b7"
                            color: "#2a7de1"
                            Text { anchors.centerIn: parent; text: "确定"; color: "#ffffff"; font.pixelSize: 12 }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    runConnectionDialog.accept()
                                    appState.connectToDevice(runConnectHost.text, runConnectPortValue)
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: instrumentControlDialog
        title: "仪器控制"
        modal: true
        width: 760
        height: 520
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onOpened: root.instrumentControlTabIndex = 0
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                AppTabHeader {
                    Layout.fillWidth: true
                    height: 30
                    labels: ["温度控制", "真空泵控制"]
                    currentIndex: root.instrumentControlTabIndex
                    onSelected: function(index) { root.instrumentControlTabIndex = index }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.instrumentControlTabIndex

                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: "#d0d5dd"

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 8

                                    RowLayout {
                                        Layout.leftMargin: 146
                                        spacing: 50
                                        Label { text: "当前值"; font.pixelSize: 14; font.bold: true }
                                        Label { text: "设定值"; font.pixelSize: 14; font.bold: true }
                                    }

                                    Repeater {
                                        model: [
                                            { label: "进样管路1[℃]", setV: 150.0 },
                                            { label: "进样管路2[℃]", setV: 150.0 },
                                            { label: "进样腔体[℃]：", setV: 80.0 },
                                            { label: "真空腔体[℃]：", setV: 80.0 }
                                        ]
                                        delegate: RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 8
                                            Label { text: modelData.label; font.pixelSize: 12; Layout.preferredWidth: 150; horizontalAlignment: Text.AlignRight }
                                            TextField { text: ""; implicitWidth: 110; implicitHeight: 34; enabled: false }
                                            SpinBox {
                                                from: 0
                                                to: 300
                                                value: modelData.setV
                                                editable: true
                                                implicitWidth: 120
                                                implicitHeight: 34
                                            }
                                            Button { text: "设置"; implicitWidth: 110; implicitHeight: 34 }
                                            Item { Layout.fillWidth: true }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            GroupBox {
                                title: "分子泵"
                                Layout.fillWidth: true
                                Layout.preferredHeight: 112
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 28
                                    Button {
                                        text: "高转速"
                                        implicitWidth: 130
                                        implicitHeight: 42
                                        highlighted: true
                                    }
                                    Button {
                                        text: "低转速"
                                        implicitWidth: 130
                                        implicitHeight: 42
                                    }
                                    Item { Layout.fillWidth: true }
                                }
                            }
                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: scanSettingsDialog
        title: "扫描设置"
        modal: true
        width: 780
        height: 760
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onAccepted: appState.toastRequested("扫描设置已应用")
        onOpened: root.scanTabIndex = 0
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: "#f5f6f8"
                    border.color: "#d0d5dd"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 2
                        spacing: 4

                        Rectangle {
                            Layout.preferredWidth: 100
                            Layout.fillHeight: true
                            color: root.scanTabIndex === 0 ? "#ffffff" : "#f5f6f8"
                            border.color: "#cfd4dc"
                            Text {
                                anchors.centerIn: parent
                                text: "扫描参数"
                                font.pixelSize: 16
                                font.bold: true
                                color: "#1f2937"
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.scanTabIndex = 0
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 90
                            Layout.fillHeight: true
                            color: root.scanTabIndex === 1 ? "#ffffff" : "#f5f6f8"
                            border.color: "#cfd4dc"
                            Text {
                                anchors.centerIn: parent
                                text: "质量轴"
                                font.pixelSize: 16
                                font.bold: true
                                color: "#1f2937"
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.scanTabIndex = 1
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.scanTabIndex

                    ScrollView {
                        clip: true
                        ColumnLayout {
                            width: parent.width - 14
                            spacing: 8

                            GroupBox {
                                title: "扫描方式"
                                Layout.fillWidth: true
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 36
                                        RadioButton {
                                            text: "全扫描"
                                            checked: root.scanFullMode
                                            onToggled: if (checked) root.scanFullMode = true
                                            font.pixelSize: 12
                                            indicator: Rectangle {
                                                implicitWidth: 13
                                                implicitHeight: 13
                                                radius: 7
                                                border.color: "#b8c0cc"
                                                color: "#ffffff"
                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    width: 7
                                                    height: 7
                                                    radius: 4
                                                    color: "#2a7de1"
                                                    visible: parent.parent.checked
                                                }
                                            }
                                        }
                                        RadioButton {
                                            text: "选择离子扫描"
                                            checked: !root.scanFullMode
                                            onToggled: if (checked) root.scanFullMode = false
                                            font.pixelSize: 12
                                            indicator: Rectangle {
                                                implicitWidth: 13
                                                implicitHeight: 13
                                                radius: 7
                                                border.color: "#b8c0cc"
                                                color: "#ffffff"
                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    width: 7
                                                    height: 7
                                                    radius: 4
                                                    color: "#2a7de1"
                                                    visible: parent.parent.checked
                                                }
                                            }
                                        }
                                        Item { Layout.fillWidth: true }
                                    }
                                }
                            }

                            GroupBox {
                                title: "质量轴设置"
                                Layout.fillWidth: true
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Label { text: "k" }
                                    TextField { implicitWidth: 120; implicitHeight: 30; text: "432.6099" }
                                    Label { text: "b" }
                                    TextField { implicitWidth: 120; implicitHeight: 30; text: "-0.4011911" }
                                    CheckBox {
                                        text: "使用质量轴"
                                        checked: root.scanUseMassAxis
                                        onToggled: root.scanUseMassAxis = checked
                                        font.pixelSize: 12
                                        scale: 0.9
                                    }
                                    Button { text: "全部应用"; implicitWidth: 96; implicitHeight: 32 }
                                    Item { Layout.fillWidth: true }
                                }
                            }

                            GroupBox {
                                title: "目标离子"
                                Layout.fillWidth: true
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "质量数:"; Layout.preferredWidth: 72 }
                                        CheckBox {
                                            text: "69"
                                            checked: true
                                            enabled: false
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        CheckBox {
                                            text: "131"
                                            checked: true
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        CheckBox {
                                            text: "219"
                                            checked: true
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        CheckBox {
                                            text: "264"
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        CheckBox {
                                            text: "414"
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        CheckBox {
                                            text: "502"
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        Item { Layout.fillWidth: true }
                                    }

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "电压:"; Layout.preferredWidth: 72 }
                                        Repeater {
                                            model: 6
                                            delegate: TextField {
                                                implicitWidth: 72
                                                implicitHeight: 30
                                                text: "1"
                                                enabled: false
                                            }
                                        }
                                        Item { Layout.fillWidth: true }
                                    }
                                }
                            }

                            GroupBox {
                                title: "全扫描设置"
                                Layout.fillWidth: true
                                visible: root.scanFullMode
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "质量数:"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "1" }
                                        Label { text: "-" }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "351" }
                                        Label { text: "m/z" }
                                        Item { Layout.fillWidth: true }
                                    }

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "扫描速度:"; Layout.preferredWidth: 72 }
                                        ComboBox { model: ["1k", "2k", "5k", "10k"]; currentIndex: 0; implicitWidth: 120; implicitHeight: 30 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "1000"; enabled: false }
                                        Label { text: "amu/s" }
                                        Item { Layout.fillWidth: true }
                                    }

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "扫描时间:"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "350" }
                                        CheckBox {
                                            text: "自定时间"
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        Label { text: "ms" }
                                        Item { Layout.fillWidth: true }
                                    }

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "回扫时间:"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "130" }
                                        Label { text: "ms" }
                                        Item { Layout.fillWidth: true }
                                    }

                                    RowLayout {
                                        spacing: 8
                                        CheckBox {
                                            text: "使用电压"
                                            checked: root.scanUseTargetVoltage
                                            onToggled: root.scanUseTargetVoltage = checked
                                            font.pixelSize: 12
                                            scale: 0.9
                                        }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "0.003238"; enabled: root.scanUseTargetVoltage }
                                        Label { text: "-" }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "0.812281"; enabled: root.scanUseTargetVoltage }
                                        Label { text: "V" }
                                        Item { Layout.fillWidth: true }
                                        Button { text: "计算"; implicitWidth: 90; implicitHeight: 32; enabled: root.scanUseTargetVoltage }
                                    }
                                }
                            }

                            GroupBox {
                                title: "选择离子扫描设置"
                                Layout.fillWidth: true
                                visible: !root.scanFullMode
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "驻留时间:"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "60" }
                                        Label { text: "ms" }
                                        Item { Layout.fillWidth: true }
                                    }
                                    RowLayout {
                                        spacing: 8
                                        Label { text: "回扫时间:"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "50" }
                                        Label { text: "ms" }
                                        Item { Layout.fillWidth: true }
                                    }
                                    RowLayout {
                                        spacing: 8
                                        Label { text: "目标峰宽度"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "15" }
                                        Label { text: "amu" }
                                        Item { Layout.fillWidth: true }
                                    }
                                    RowLayout {
                                        spacing: 8
                                        Label { text: "斜扫电压:"; Layout.preferredWidth: 72 }
                                        TextField { implicitWidth: 120; implicitHeight: 30; text: "0.011464"; enabled: false }
                                        Label { text: "V" }
                                        Item { Layout.fillWidth: true }
                                    }
                                    RowLayout {
                                        Item { Layout.fillWidth: true }
                                        Button { text: "计算"; implicitWidth: 90; implicitHeight: 32 }
                                    }
                                }
                            }

                            GroupBox {
                                title: "RFAD"
                                Layout.fillWidth: true
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Label { text: "采样频率:"; Layout.preferredWidth: 72 }
                                    ComboBox { model: ["10k", "20k", "50k", "100k"]; currentIndex: 1; implicitWidth: 120; implicitHeight: 30 }
                                    Label { text: "k/s" }
                                    Item { Layout.fillWidth: true }
                                    Button { text: "设置"; implicitWidth: 90; implicitHeight: 32; onClicked: appState.toastRequested("RFAD 设置已更新") }
                                }
                            }
                        }
                    }

                    ScrollView {
                        clip: true
                        ColumnLayout {
                            width: parent.width - 14
                            spacing: 8
                            GroupBox {
                                title: ""
                                Layout.fillWidth: true
                                background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 10

                                    RowLayout {
                                        Layout.leftMargin: 44
                                        spacing: 62
                                        Label { text: "质量数"; font.bold: true }
                                        Label { text: "电压值"; font.bold: true }
                                    }

                                    Repeater {
                                        model: [
                                            { mz: "14", voltage: "0.05055", checked: true },
                                            { mz: "28", voltage: "0.09658", checked: true },
                                            { mz: "32", voltage: "0.10952", checked: true }
                                        ]
                                        delegate: RowLayout {
                                            spacing: 8
                                            CheckBox {
                                                checked: modelData.checked
                                                font.pixelSize: 12
                                                scale: 0.9
                                            }
                                            TextField { implicitWidth: 160; implicitHeight: 34; text: modelData.mz }
                                            TextField { implicitWidth: 160; implicitHeight: 34; text: modelData.voltage }
                                            Item { Layout.fillWidth: true }
                                        }
                                    }

                                    RowLayout {
                                        spacing: 24
                                        Layout.leftMargin: 46
                                        Button { text: "计算"; implicitWidth: 90; implicitHeight: 34 }
                                        Button { text: "应用"; implicitWidth: 90; implicitHeight: 34 }
                                        Item { Layout.fillWidth: true }
                                    }

                                    Label { text: "质量轴"; Layout.leftMargin: 46; font.pixelSize: 16 }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 54
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#f5f6f8"
                            border.color: "#b9c1cb"
                            Text {
                                anchors.centerIn: parent
                                text: "取消"
                                font.pixelSize: 12
                                color: "#334155"
                            }
                            MouseArea { anchors.fill: parent; onClicked: scanSettingsDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#2a7de1"
                            border.color: "#1d64b7"
                            Text {
                                anchors.centerIn: parent
                                text: "确定"
                                font.pixelSize: 12
                                color: "#ffffff"
                            }
                            MouseArea { anchors.fill: parent; onClicked: scanSettingsDialog.accept() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: dataProcessingDialog
        title: "数据处理"
        modal: true
        width: 460
        height: 500
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                GroupBox {
                    title: "滤波"
                    Layout.fillWidth: true
                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        columns: 2
                        rowSpacing: 8
                        columnSpacing: 8

                        Label {
                            text: "低分界频率 [Hz]："
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: 180
                        }
                        TextField {
                            text: "3000"
                            implicitWidth: 180
                            implicitHeight: 36
                            font.pixelSize: 16
                        }

                        Label {
                            text: "叠加次数："
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: 180
                        }
                        TextField {
                            text: "3"
                            implicitWidth: 180
                            implicitHeight: 36
                            font.pixelSize: 16
                        }
                    }
                }

                GroupBox {
                    title: "基线"
                    Layout.fillWidth: true
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        RowLayout {
                            spacing: 8
                            Label {
                                text: "手动基线："
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignRight
                                Layout.preferredWidth: 180
                            }
                            TextField {
                                text: "1"
                                implicitWidth: 180
                                implicitHeight: 36
                                font.pixelSize: 16
                            }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            spacing: 8
                            Item { Layout.preferredWidth: 90 }
                            CheckBox { text: "自动计算基线"; font.pixelSize: 12 }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "确定"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: dataProcessingDialog.accept() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "取消"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: dataProcessingDialog.reject() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: systemSettingsDialog
        title: "系统设置"
        modal: true
        width: 620
        height: 500
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onOpened: root.systemSettingsTabIndex = 0
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                AppTabHeader {
                    Layout.fillWidth: true
                    height: 30
                    labels: ["扫描配置", "版本配置"]
                    currentIndex: root.systemSettingsTabIndex
                    onSelected: function(index) { root.systemSettingsTabIndex = index }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.systemSettingsTabIndex

                    Rectangle {
                        color: "#ffffff"
                        border.color: "#d0d5dd"
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8

                            GroupBox {
                                title: "RFDA"
                                Layout.fillWidth: true
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Label { text: "RFDDS："; font.pixelSize: 12; Layout.preferredWidth: 88 }
                                    TextField { text: "1250000"; implicitWidth: 220; implicitHeight: 36; font.pixelSize: 16 }
                                    Item { Layout.fillWidth: true }
                                    Button { text: "设置"; implicitWidth: 90; implicitHeight: 36 }
                                }
                            }

                            GroupBox {
                                title: "AD"
                                Layout.fillWidth: true
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "频率："; font.pixelSize: 12; Layout.preferredWidth: 88 }
                                        ComboBox { model: ["80K", "120K", "160K", "200K"]; currentIndex: 2; implicitWidth: 220; implicitHeight: 36; font.pixelSize: 16 }
                                        Label { text: "k/s"; font.pixelSize: 12 }
                                        Item { Layout.fillWidth: true }
                                        Button { text: "设置"; implicitWidth: 90; implicitHeight: 36 }
                                    }

                                    RowLayout {
                                        spacing: 8
                                        Label { text: "比例："; font.pixelSize: 12; Layout.preferredWidth: 88 }
                                        TextField { text: "20"; implicitWidth: 220; implicitHeight: 36; font.pixelSize: 16 }
                                        CheckBox { text: "使用固定比例"; font.pixelSize: 12 }
                                        Item { Layout.fillWidth: true }
                                    }
                                }
                            }

                            GroupBox {
                                title: "扫描基线"
                                Layout.fillWidth: true
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8
                                    Label { text: "质量数："; font.pixelSize: 12; Layout.preferredWidth: 88 }
                                    TextField { text: "3"; implicitWidth: 220; implicitHeight: 36; font.pixelSize: 16 }
                                    Label { text: "amu"; font.pixelSize: 12 }
                                    Item { Layout.fillWidth: true }
                                    Button { text: "设置"; implicitWidth: 90; implicitHeight: 36 }
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }

                    Rectangle {
                        color: "#ffffff"
                        border.color: "#d0d5dd"
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8

                            GroupBox {
                                title: "系统信息"
                                Layout.fillWidth: true
                                GridLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    columns: 3
                                    rowSpacing: 8
                                    columnSpacing: 8

                                    Label { text: "编号："; font.pixelSize: 12; Layout.preferredWidth: 88; horizontalAlignment: Text.AlignRight }
                                    TextField { text: "1001"; implicitWidth: 220; implicitHeight: 36; font.pixelSize: 16 }
                                    Item { Layout.fillWidth: true }

                                    Label { text: "型号："; font.pixelSize: 12; Layout.preferredWidth: 88; horizontalAlignment: Text.AlignRight }
                                    ComboBox { model: ["Z16", "Z18", "Z20"]; currentIndex: 1; implicitWidth: 220; implicitHeight: 36; font.pixelSize: 16 }
                                    Button { text: "设置"; implicitWidth: 90; implicitHeight: 36 }
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 0
                    color: "transparent"
                    border.color: "#d0d5dd"
                }
            }
        }
    }

    Dialog {
        id: chartSettingsDialog
        title: "谱图设置"
        modal: true
        width: 900
        height: 420
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onAccepted: appState.toastRequested("谱图设置已应用")
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ffffff"
                    border.color: "#d0d5dd"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        ColumnLayout {
                            Layout.preferredWidth: 170
                            Layout.fillHeight: true
                            spacing: 14
                            RadioButton {
                                text: "质量数"
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            RadioButton {
                                text: "电压"
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            RadioButton {
                                text: "时间"
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            RadioButton {
                                text: "点数"
                                checked: true
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            Item { Layout.fillHeight: true }
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: "#d0d5dd"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 12
                            AppCheckBox { text: "谱图不自动恢复全屏" }
                            AppCheckBox { text: "Y轴范围固定"; checked: root.chartFixedY; onToggled: root.chartFixedY = checked }
                            Item { Layout.preferredHeight: 8 }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "最小值"; Layout.preferredWidth: 56 }
                                TextField { Layout.fillWidth: true; implicitHeight: 40; enabled: root.chartFixedY; text: "0.0" }
                            }
                            Item { Layout.preferredHeight: 10 }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "最大值"; Layout.preferredWidth: 56 }
                                TextField { Layout.fillWidth: true; implicitHeight: 40; enabled: root.chartFixedY; text: "1000.0" }
                            }
                            Item { Layout.fillHeight: true }
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: "#d0d5dd"
                        }

                        ColumnLayout {
                            Layout.preferredWidth: 260
                            Layout.fillHeight: true
                            spacing: 12
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "谱峰个数"; Layout.preferredWidth: 70 }
                                SpinBox { from: 0; to: 999; value: 8; editable: true; implicitHeight: 40; implicitWidth: 140 }
                            }
                            AppCheckBox { text: "显示原始数据"; checked: true }
                            AppCheckBox { text: "显示余晖" }
                            AppCheckBox { text: "显示半峰宽" }
                            Item { Layout.fillHeight: true }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 54
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#f5f6f8"
                            border.color: "#d0d5dd"
                            Text { anchors.centerIn: parent; text: "取消"; font.pixelSize: 12; color: "#334155" }
                            MouseArea { anchors.fill: parent; onClicked: chartSettingsDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#2a7de1"
                            border.color: "#1d64b7"
                            Text { anchors.centerIn: parent; text: "确定"; font.pixelSize: 12; color: "#ffffff" }
                            MouseArea { anchors.fill: parent; onClicked: chartSettingsDialog.accept() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 34
        z: 100
        visible: false
        color: "#f5f7fa"
        border.color: "#d0d5dd"

        RowLayout {
            id: topMenuRow
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 2

            Repeater {
                model: [
                    { label: "文件" },
                    { label: "运行" },
                    { label: "设置" },
                    { label: "视图" }
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.preferredWidth: 58
                    Layout.preferredHeight: 24
                    radius: 2
                    color: menuMouse.containsMouse ? "#e7edf5" : "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        font.pixelSize: 13
                        color: "#101828"
                    }
                    MouseArea {
                        id: menuMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {}
                    }
                }
            }
            Item { Layout.fillWidth: true }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.topMargin: 0
        spacing: 0

        Rectangle {
            Layout.preferredWidth: ThemeLib.Theme.size.navW
            Layout.fillHeight: true
            color: "#dfe4ea"
            border.color: "#c9d0da"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 38; color: "transparent" }

                Repeater {
                    model: ["调谐", "监测", "方法", "数据", "设置"]
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 62
                        color: index === root.currentPageIndex ? "#d7e6ff" : "transparent"
                        border.width: index === root.currentPageIndex ? 1 : 0
                        border.color: "#98b8f2"

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: 14
                            color: index === root.currentPageIndex ? "#2d7ef7" : "#4b5563"
                            font.bold: index === root.currentPageIndex
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.currentPageIndex = index
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ThemeLib.Theme.color.workspaceBg

            StackLayout {
                anchors.fill: parent
                anchors.margins: 8
                currentIndex: root.currentPageIndex

                Item {
                    id: tunePage
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 31
                            color: ThemeLib.Theme.color.toolbarBg
                            border.color: ThemeLib.Theme.color.borderStrong

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 3
                                spacing: 4

                                AppButton {
                                    text: "开始"
                                    selected: appState.scanning
                                    enabled: appState.connected && !appState.scanning
                                    onClicked: appState.startScan(tuneMassStart.value, tuneMassEnd.value)
                                }
                                AppButton {
                                    text: "停止"
                                    selected: !appState.scanning
                                    enabled: appState.connected && appState.scanning
                                    onClicked: appState.stopScan()
                                }
                                AppButton {
                                    text: "校正当前质量轴"
                                    implicitWidth: 102
                                    onClicked: {
                                        if (!root.requireConnected()) return
                                        appState.calibrateMassAxis()
                                    }
                                }
                                AppButton {
                                    text: tuneForePumpOn ? "前级泵✓" : "前级泵"
                                    selected: tuneForePumpOn
                                    onClicked: {
                                        tuneForePumpOn = !tuneForePumpOn
                                        appState.setInstrumentSwitch("forePump", tuneForePumpOn)
                                    }
                                }
                                AppButton {
                                    text: tuneForeValveOn ? "前级阀✓" : "前级阀"
                                    selected: tuneForeValveOn
                                    onClicked: {
                                        tuneForeValveOn = !tuneForeValveOn
                                        appState.setInstrumentSwitch("foreValve", tuneForeValveOn)
                                    }
                                }
                                AppButton {
                                    text: tuneMolecularPumpOn ? "分子泵✓" : "分子泵"
                                    selected: tuneMolecularPumpOn
                                    onClicked: {
                                        tuneMolecularPumpOn = !tuneMolecularPumpOn
                                        appState.setInstrumentSwitch("molecularPump", tuneMolecularPumpOn)
                                    }
                                }
                                AppButton {
                                    text: tuneInletValveOn ? "进样阀✓" : "进样阀"
                                    selected: tuneInletValveOn
                                    onClicked: {
                                        tuneInletValveOn = !tuneInletValveOn
                                        appState.setInstrumentSwitch("inletValve", tuneInletValveOn)
                                    }
                                }
                                AppButton {
                                    text: tuneFilamentOn ? "灯丝✓" : "灯丝"
                                    selected: tuneFilamentOn
                                    onClicked: {
                                        tuneFilamentOn = !tuneFilamentOn
                                        appState.setInstrumentSwitch("filament", tuneFilamentOn)
                                    }
                                }
                                AppButton {
                                    text: tuneMultiplierOn ? "倍增器✓" : "倍增器"
                                    selected: tuneMultiplierOn
                                    onClicked: {
                                        tuneMultiplierOn = !tuneMultiplierOn
                                        appState.setInstrumentSwitch("multiplier", tuneMultiplierOn)
                                    }
                                }
                                AppButton {
                                    text: "灯丝切换"
                                    implicitWidth: 70
                                    onClicked: {
                                        appState.toggleDetectorMode()
                                    }
                                }
                                AppButton {
                                    text: "数据保存"
                                    implicitWidth: 66
                                    onClicked: {
                                        if (!root.requireConnected()) return
                                        appState.saveCurrentFrame()
                                    }
                                }
                                AppButton {
                                    text: "谱图刷新"
                                    implicitWidth: 66
                                    onClicked: appState.refreshCharts()
                                }
                                AppButton {
                                    text: "AI总结"
                                    implicitWidth: 66
                                    enabled: !tuneAiBusy
                                    onClicked: tuneAiOpenSummaryDialog()
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#f4f6f8"
                                border.color: ThemeLib.Theme.color.borderDefault

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 6

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 146
                                        color: "#eff2f6"
                                        border.color: ThemeLib.Theme.color.borderDefault

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 4
                                            spacing: 4

                                            Rectangle {
                                                Layout.preferredWidth: 236
                                                Layout.fillHeight: true
                                                color: "#ffffff"
                                                border.color: ThemeLib.Theme.color.borderSubtle

                                                ColumnLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 8
                                                    spacing: 6

                                                    RowLayout {
                                                        ButtonGroup { id: tuneScanModeGroup }
                                                        RadioButton {
                                                            id: tuneFullScanRadio
                                                            text: "Full Scan"
                                                            font.pixelSize: 12
                                                            spacing: 4
                                                            topPadding: 0
                                                            bottomPadding: 0
                                                            implicitHeight: 22
                                                            Layout.alignment: Qt.AlignVCenter
                                                            indicator: Rectangle {
                                                                y: (tuneFullScanRadio.height - height) / 2
                                                                implicitWidth: 12
                                                                implicitHeight: 12
                                                                radius: 6
                                                                border.width: 1
                                                                border.color: "#98a2b3"
                                                                color: "transparent"
                                                                Rectangle {
                                                                    anchors.centerIn: parent
                                                                    width: 6
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#2563eb"
                                                                    visible: tuneFullScanRadio.checked
                                                                }
                                                            }
                                                            checked: root.scanFullMode
                                                            ButtonGroup.group: tuneScanModeGroup
                                                            onToggled: if (checked) root.scanFullMode = true
                                                        }
                                                        RadioButton {
                                                            id: tuneSimRadio
                                                            text: "SIM"
                                                            font.pixelSize: 12
                                                            spacing: 4
                                                            topPadding: 0
                                                            bottomPadding: 0
                                                            implicitHeight: 22
                                                            Layout.alignment: Qt.AlignVCenter
                                                            indicator: Rectangle {
                                                                y: (tuneSimRadio.height - height) / 2
                                                                implicitWidth: 12
                                                                implicitHeight: 12
                                                                radius: 6
                                                                border.width: 1
                                                                border.color: "#98a2b3"
                                                                color: "transparent"
                                                                Rectangle {
                                                                    anchors.centerIn: parent
                                                                    width: 6
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#2563eb"
                                                                    visible: tuneSimRadio.checked
                                                                }
                                                            }
                                                            checked: !root.scanFullMode
                                                            ButtonGroup.group: tuneScanModeGroup
                                                            onToggled: if (checked) root.scanFullMode = false
                                                        }
                                                        Item { Layout.fillWidth: true }
                                                        AppButton { text: "设置"; implicitWidth: 46 }
                                                    }

                                                    GridLayout {
                                                        columns: 2
                                                        rowSpacing: 4
                                                        columnSpacing: 8
                                                        Label { text: "起始质量数："; font.pixelSize: 12 }
                                                        SpinBox {
                                                            id: tuneMassStart
                                                            from: 1
                                                            to: 500
                                                            value: 10
                                                            editable: true
                                                            implicitWidth: 120
                                                            implicitHeight: 24
                                                            leftPadding: 20
                                                            rightPadding: 20
                                                            down.indicator: Rectangle {
                                                                x: 0
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassStart.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                            up.indicator: Rectangle {
                                                                x: tuneMassStart.width - width
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassStart.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                        }
                                                        Label { text: "终止质量数："; font.pixelSize: 12 }
                                                        SpinBox {
                                                            id: tuneMassEnd
                                                            from: 1
                                                            to: 500
                                                            value: 110
                                                            editable: true
                                                            implicitWidth: 120
                                                            implicitHeight: 24
                                                            leftPadding: 20
                                                            rightPadding: 20
                                                            down.indicator: Rectangle {
                                                                x: 0
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassEnd.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                            up.indicator: Rectangle {
                                                                x: tuneMassEnd.width - width
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassEnd.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                Layout.preferredWidth: 236
                                                Layout.fillHeight: true
                                                color: "#ffffff"
                                                border.color: ThemeLib.Theme.color.borderSubtle

                                                ColumnLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 8
                                                    spacing: 6
                                                    Label { text: "检测器"; font.pixelSize: 12; font.bold: true }
                                                    RowLayout {
                                                        ButtonGroup { id: tuneDetectorGroup }
                                                        RadioButton {
                                                            id: tuneDetectorMultiplierRadio
                                                            text: "倍增器"
                                                            font.pixelSize: 12
                                                            spacing: 4
                                                            topPadding: 0
                                                            bottomPadding: 0
                                                            implicitHeight: 22
                                                            Layout.alignment: Qt.AlignVCenter
                                                            indicator: Rectangle {
                                                                y: (tuneDetectorMultiplierRadio.height - height) / 2
                                                                implicitWidth: 12
                                                                implicitHeight: 12
                                                                radius: 6
                                                                border.width: 1
                                                                border.color: "#98a2b3"
                                                                color: "transparent"
                                                                Rectangle {
                                                                    anchors.centerIn: parent
                                                                    width: 6
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#2563eb"
                                                                    visible: tuneDetectorMultiplierRadio.checked
                                                                }
                                                            }
                                                            checked: true
                                                            ButtonGroup.group: tuneDetectorGroup
                                                        }
                                                        RadioButton {
                                                            id: tuneDetectorFaradayRadio
                                                            text: "法拉第筒"
                                                            font.pixelSize: 12
                                                            spacing: 4
                                                            topPadding: 0
                                                            bottomPadding: 0
                                                            implicitHeight: 22
                                                            Layout.alignment: Qt.AlignVCenter
                                                            indicator: Rectangle {
                                                                y: (tuneDetectorFaradayRadio.height - height) / 2
                                                                implicitWidth: 12
                                                                implicitHeight: 12
                                                                radius: 6
                                                                border.width: 1
                                                                border.color: "#98a2b3"
                                                                color: "transparent"
                                                                Rectangle {
                                                                    anchors.centerIn: parent
                                                                    width: 6
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#2563eb"
                                                                    visible: tuneDetectorFaradayRadio.checked
                                                                }
                                                            }
                                                            ButtonGroup.group: tuneDetectorGroup
                                                        }
                                                    }
                                                    Label { text: "稳定区"; font.pixelSize: 12; font.bold: true }
                                                    RowLayout {
                                                        ButtonGroup { id: tuneZoneGroup }
                                                        RadioButton {
                                                            id: tuneZone1Radio
                                                            text: "稳定区一"
                                                            font.pixelSize: 12
                                                            spacing: 4
                                                            topPadding: 0
                                                            bottomPadding: 0
                                                            implicitHeight: 22
                                                            Layout.alignment: Qt.AlignVCenter
                                                            indicator: Rectangle {
                                                                y: (tuneZone1Radio.height - height) / 2
                                                                implicitWidth: 12
                                                                implicitHeight: 12
                                                                radius: 6
                                                                border.width: 1
                                                                border.color: "#98a2b3"
                                                                color: "transparent"
                                                                Rectangle {
                                                                    anchors.centerIn: parent
                                                                    width: 6
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#2563eb"
                                                                    visible: tuneZone1Radio.checked
                                                                }
                                                            }
                                                            checked: true
                                                            ButtonGroup.group: tuneZoneGroup
                                                        }
                                                        RadioButton {
                                                            id: tuneZone2Radio
                                                            text: "稳定区二"
                                                            font.pixelSize: 12
                                                            spacing: 4
                                                            topPadding: 0
                                                            bottomPadding: 0
                                                            implicitHeight: 22
                                                            Layout.alignment: Qt.AlignVCenter
                                                            indicator: Rectangle {
                                                                y: (tuneZone2Radio.height - height) / 2
                                                                implicitWidth: 12
                                                                implicitHeight: 12
                                                                radius: 6
                                                                border.width: 1
                                                                border.color: "#98a2b3"
                                                                color: "transparent"
                                                                Rectangle {
                                                                    anchors.centerIn: parent
                                                                    width: 6
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#2563eb"
                                                                    visible: tuneZone2Radio.checked
                                                                }
                                                            }
                                                            ButtonGroup.group: tuneZoneGroup
                                                        }
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                color: "#ffffff"
                                                border.color: ThemeLib.Theme.color.borderSubtle

                                                ColumnLayout {
                                                    id: tuneSummaryTable
                                                    anchors.fill: parent
                                                    anchors.margins: 4
                                                    spacing: 0

                                                    property var rows: [
                                                        ["m/z", "18", "28", "44"],
                                                        ["强度[v]", f1(appState.peak18), f1(appState.peak28), f1(appState.peak44)],
                                                        ["丰度比[%]", f1(appState.peak18 / 20.0), f1(appState.peak28 / 20.0), f1(appState.peak44 / 20.0)],
                                                        ["半峰宽[m.z]", f2(1.20), f2(1.40), f2(1.60)]
                                                    ]

                                                    Repeater {
                                                        model: tuneSummaryTable.rows.length
                                                        delegate: RowLayout {
                                                            property int rowIndex: index
                                                            Layout.fillWidth: true
                                                            Layout.preferredHeight: 28
                                                            spacing: 0

                                                            Repeater {
                                                                model: 4
                                                                delegate: Rectangle {
                                                                    Layout.fillWidth: true
                                                                    Layout.preferredWidth: index === 0 ? 1.3 : 1.0
                                                                    Layout.fillHeight: true
                                                                    border.color: "#d8dde5"
                                                                    color: rowIndex === 0 ? "#f6f8fb" : "#ffffff"

                                                                    Text {
                                                                        anchors.centerIn: parent
                                                                        text: tuneSummaryTable.rows[rowIndex][index]
                                                                        font.pixelSize: 12
                                                                        font.bold: rowIndex === 0
                                                                        color: ThemeLib.Theme.color.textPrimary
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        color: "#ffffff"
                                        border.color: ThemeLib.Theme.color.borderDefault

                                        Loader {
                                            anchors.fill: parent
                                            active: !root.isWasm
                                            sourceComponent: tuneChartsComponent
                                        }

                                        Component {
                                            id: tuneChartsComponent
                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 3
                                                spacing: 3

                                                ChartView {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    Layout.preferredHeight: 4.35
                                                    antialiasing: true
                                                    legend.visible: false
                                                    title: "全扫描"
                                                    backgroundColor: ThemeLib.Theme.color.chartBg
                                                    plotAreaColor: ThemeLib.Theme.color.chartBg
                                                    ValueAxis { id: sx; min: 10; max: 110 }
                                                    ValueAxis { id: sy; min: 0; max: 1000 }
                                                    LineSeries {
                                                        id: spectrumSeries
                                                        axisX: sx
                                                        axisY: sy
                                                        color: ThemeLib.Theme.color.linePrimary
                                                        width: 2
                                                    }
                                                    Component.onCompleted: appState.bindSpectrumSeries(spectrumSeries)
                                                }

                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    height: 1
                                                    color: "#cfd5de"
                                                }

                                                ChartView {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    Layout.preferredHeight: 1.55
                                                    antialiasing: true
                                                    legend.visible: false
                                                    title: "TIC"
                                                    backgroundColor: ThemeLib.Theme.color.chartBg
                                                    plotAreaColor: ThemeLib.Theme.color.chartBg
                                                    ValueAxis { id: tuneTx; min: 0; max: 240 }
                                                    ValueAxis { id: tuneTy; min: 0; max: 250000 }
                                                    LineSeries {
                                                        id: tuneTicSeries
                                                        axisX: tuneTx
                                                        axisY: tuneTy
                                                        color: ThemeLib.Theme.color.linePrimary
                                                        width: 2
                                                    }
                                                    Component.onCompleted: appState.bindTicSeries(tuneTicSeries)
                                                }
                                            }
                                        }

                                        Rectangle {
                                            anchors.fill: parent
                                            visible: root.isWasm
                                            color: "#f4f6f9"
                                            border.color: "#cfd5de"
                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                spacing: 6
                                                Loader {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    Layout.preferredHeight: 4.35
                                                    sourceComponent: webLineChartComponent
                                                    onLoaded: {
                                                        item.chartTitle = "全扫描"
                                                        item.chartKey = "spectrum"
                                                        item.autoX = false
                                                        item.autoY = true
                                                        item.fixedXMin = 10
                                                        item.fixedXMax = 110
                                                    }
                                                }
                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 1
                                                    color: "#cfd5de"
                                                }
                                                Loader {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    Layout.preferredHeight: 1.55
                                                    sourceComponent: webLineChartComponent
                                                    onLoaded: {
                                                        item.chartTitle = "TIC"
                                                        item.chartKey = "tic"
                                                        item.autoX = true
                                                        item.autoY = true
                                                    }
                                                }
                                            }
                                        }

                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.leftMargin: 4
                                            anchors.bottom: parent.bottom
                                            anchors.bottomMargin: 154
                                            width: 44
                                            height: 20
                                            color: "transparent"
                                            RowLayout {
                                                anchors.fill: parent
                                                spacing: 2
                                                Rectangle {
                                                    width: 18
                                                    height: 18
                                                    color: "#f0f2f5"
                                                    border.color: "#bfc6d0"
                                                    Text { anchors.centerIn: parent; text: "◀"; font.pixelSize: 10; color: "#3f4753" }
                                                }
                                                Rectangle {
                                                    width: 18
                                                    height: 18
                                                    color: "#f0f2f5"
                                                    border.color: "#bfc6d0"
                                                    Text { anchors.centerIn: parent; text: "▶"; font.pixelSize: 10; color: "#3f4753" }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            AppPanel {
                                Layout.preferredWidth: 338
                                Layout.fillHeight: true
                                color: ThemeLib.Theme.color.panelSubtleBg

                                AppTabHeader {
                                    id: tuneTabHeader
                                    width: parent.width
                                    height: 30
                                    labels: ["参数配置", "仪器状态", "设备连接", "AI助手"]
                                    currentIndex: tuneRightStack.currentIndex
                                    onSelected: function(index) { tuneRightStack.currentIndex = index }
                                }

                                StackLayout {
                                    id: tuneRightStack
                                    anchors.top: tuneTabHeader.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            x: 8
                                            width: Math.max(0, parent.width - 16)
                                            spacing: 6

                                            AppCheckBox {
                                                text: "显示高级参数"
                                                checked: root.tuneShowAdvanced
                                                onToggled: root.tuneShowAdvanced = checked
                                            }

                                            GroupBox {
                                                title: "离子源"
                                                Layout.fillWidth: true
                                                GridLayout {
                                                    columns: 3
                                                    Label { text: "推斥极 [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: repeller
                                                        from: -200
                                                        to: 200
                                                        value: 12
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: repeller.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: repeller.width - width
                                                            y: 0
                                                            width: 18
                                                            height: repeller.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (repellerSlider.value !== value) repellerSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: repellerSlider
                                                        from: -200
                                                        to: 200
                                                        value: repeller.value
                                                        onMoved: repeller.value = value
                                                        onValueChanged: {
                                                            if (repeller.value !== value) repeller.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "透镜1(选出) [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: lens1
                                                        from: -200
                                                        to: 200
                                                        value: 6
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: lens1.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: lens1.width - width
                                                            y: 0
                                                            width: 18
                                                            height: lens1.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (lens1Slider.value !== value) lens1Slider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: lens1Slider
                                                        from: -200
                                                        to: 200
                                                        value: lens1.value
                                                        onMoved: lens1.value = value
                                                        onValueChanged: {
                                                            if (lens1.value !== value) lens1.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "透镜2(聚焦) [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: lens2
                                                        from: -200
                                                        to: 200
                                                        value: 10
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: lens2.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: lens2.width - width
                                                            y: 0
                                                            width: 18
                                                            height: lens2.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (lens2Slider.value !== value) lens2Slider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: lens2Slider
                                                        from: -200
                                                        to: 200
                                                        value: lens2.value
                                                        onMoved: lens2.value = value
                                                        onValueChanged: {
                                                            if (lens2.value !== value) lens2.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "检测器"
                                                Layout.fillWidth: true
                                                GridLayout {
                                                    columns: 3
                                                    Label { text: "高端补偿"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: highComp
                                                        from: -100
                                                        to: 100
                                                        value: 0
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: highComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: highComp.width - width
                                                            y: 0
                                                            width: 18
                                                            height: highComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (highCompSlider.value !== value) highCompSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: highCompSlider
                                                        from: -100
                                                        to: 100
                                                        value: highComp.value
                                                        onMoved: highComp.value = value
                                                        onValueChanged: {
                                                            if (highComp.value !== value) highComp.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "低端补偿"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: lowComp
                                                        from: -100
                                                        to: 100
                                                        value: 0
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: lowComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: lowComp.width - width
                                                            y: 0
                                                            width: 18
                                                            height: lowComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (lowCompSlider.value !== value) lowCompSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: lowCompSlider
                                                        from: -100
                                                        to: 100
                                                        value: lowComp.value
                                                        onMoved: lowComp.value = value
                                                        onValueChanged: {
                                                            if (lowComp.value !== value) lowComp.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "倍增器 [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: multiplierVoltage
                                                        from: 0
                                                        to: 3000
                                                        value: 1200
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: multiplierVoltage.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: multiplierVoltage.width - width
                                                            y: 0
                                                            width: 18
                                                            height: multiplierVoltage.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (multiplierSlider.value !== value) multiplierSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: multiplierSlider
                                                        from: 0
                                                        to: 3000
                                                        value: multiplierVoltage.value
                                                        onMoved: multiplierVoltage.value = value
                                                        onValueChanged: {
                                                            if (multiplierVoltage.value !== value) multiplierVoltage.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "其他"
                                                Layout.fillWidth: true
                                                visible: root.tuneShowAdvanced
                                                ColumnLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 8
                                                    spacing: 6

                                                    Repeater {
                                                        model: [
                                                            { label: "ROD [V]", min: -10, max: 10, value: 0.0 },
                                                            { label: "E [V]", min: -10, max: 10, value: 0.0 },
                                                            { label: "电子能量 [eV]", min: -200, max: 0, value: -70.0 },
                                                            { label: "灯丝电流 [uA]", min: 0, max: 1000, value: 349.9 },
                                                            { label: "外偏转透镜 [V]", min: -200, max: 200, value: -15.0 },
                                                            { label: "内偏转透镜 [V]", min: -200, max: 200, value: -0.1 },
                                                            { label: "前预四极 [V]", min: -100, max: 100, value: 0.0 },
                                                            { label: "后预四极 [V]", min: -100, max: 100, value: 0.0 }
                                                        ]
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            spacing: 8

                                                            Label {
                                                                text: modelData.label
                                                                font.pixelSize: 12
                                                                Layout.preferredWidth: 110
                                                                horizontalAlignment: Text.AlignRight
                                                            }
                                                            TextField {
                                                                id: advancedValue
                                                                text: Number(modelData.value).toFixed(1)
                                                                implicitWidth: 88
                                                                implicitHeight: 24
                                                                font.pixelSize: 12
                                                                horizontalAlignment: Text.AlignHCenter
                                                                validator: DoubleValidator { notation: DoubleValidator.StandardNotation }
                                                                onEditingFinished: {
                                                                    var parsed = Number(text)
                                                                    if (!isNaN(parsed)) {
                                                                        advancedSlider.value = Math.max(advancedSlider.from, Math.min(advancedSlider.to, parsed))
                                                                        text = Number(advancedSlider.value).toFixed(1)
                                                                    } else {
                                                                        text = Number(advancedSlider.value).toFixed(1)
                                                                    }
                                                                }
                                                            }
                                                            Slider {
                                                                id: advancedSlider
                                                                from: modelData.min
                                                                to: modelData.max
                                                                value: modelData.value
                                                                stepSize: 0.1
                                                                onMoved: advancedValue.text = Number(value).toFixed(1)
                                                                onValueChanged: {
                                                                    if (!advancedValue.activeFocus) advancedValue.text = Number(value).toFixed(1)
                                                                }
                                                                implicitHeight: 20
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 128
                                                                background: Rectangle {
                                                                    x: parent.leftPadding
                                                                    y: parent.topPadding + parent.availableHeight / 2 - 3
                                                                    width: parent.availableWidth
                                                                    height: 6
                                                                    radius: 3
                                                                    color: "#e5e7eb"
                                                                    Rectangle {
                                                                        width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                        height: parent.height
                                                                        radius: 3
                                                                        color: "#4b5563"
                                                                    }
                                                                }
                                                                handle: Rectangle {
                                                                    x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                                    width: 16
                                                                    height: 16
                                                                    radius: 8
                                                                    color: "#f5f6f7"
                                                                    border.color: "#c8ccd2"
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            AppButton {
                                                text: "应用"
                                                implicitWidth: 70
                                                onClicked: appState.applyTune(repeller.value, lens1.value, lens2.value)
                                            }
                                        }
                                    }

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 10
                                            anchors.margins: 8

                                            GroupBox {
                                                title: "部件温度[℃]"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 10
                                                    spacing: 8
                                                    Repeater {
                                                        model: ["进样管路1", "进样管路2", "进样腔体", "真空腔体"]
                                                        delegate: ColumnLayout {
                                                            spacing: 4
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "0"
                                                                font.pixelSize: 13
                                                                font.bold: true
                                                                color: "#344054"
                                                            }
                                                            Rectangle {
                                                                Layout.alignment: Qt.AlignHCenter
                                                                width: 20
                                                                height: 156
                                                                radius: 3
                                                                border.color: "#c8cdd6"
                                                                color: "#f7f8fa"
                                                                Rectangle {
                                                                    anchors.left: parent.left
                                                                    anchors.right: parent.right
                                                                    anchors.bottom: parent.bottom
                                                                    height: appState.connected ? 36 : 0
                                                                    radius: 2
                                                                    color: "#57a5ff"
                                                                }
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "100"
                                                                font.pixelSize: 12
                                                                color: "#98a2b3"
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: modelData
                                                                wrapMode: Text.WordWrap
                                                                font.pixelSize: 12
                                                                color: "#475467"
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "部件状态"
                                                Layout.fillWidth: true

                                                ColumnLayout {
                                                    width: parent.width
                                                    spacing: 0
                                                    property var names: [
                                                        "+24V", "灯丝电流[A]", "电子能量[eV]", "发射电流[μA]", "射频扫描电压[V]", "射频电流[A]",
                                                        "倍增器电压[V]", "前预四极[V]", "打拿极[V]", "推斥极[V]", "透镜1[V]", "透镜2[V]"
                                                    ]
                                                    property var values: [
                                                        appState.connected ? "24" : "0",
                                                        "0.0", "0.0", "0.0",
                                                        "100.0", "20.0",
                                                        "1200.0", "0.0", "0.0", "0.0", "0.0", "0.0"
                                                    ]

                                                    Repeater {
                                                        model: 12
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            Layout.preferredHeight: 28
                                                            spacing: 0
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.45
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.left: parent.left
                                                                    anchors.leftMargin: 8
                                                                    anchors.verticalCenter: parent.verticalCenter
                                                                    text: parent.parent.parent.names[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.0
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.centerIn: parent
                                                                    text: parent.parent.parent.values[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Rectangle {
                                        color: "#f6f6f7"
                                        border.color: "#c9ccd2"
                                        radius: 2

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 8

                                            Label {
                                                text: "IP地址"
                                                font.pixelSize: 12
                                                color: "#344054"
                                            }
                                            TextField {
                                                id: tuneHostField
                                                Layout.fillWidth: true
                                                implicitHeight: 28
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: appState.connectionHost
                                            }

                                            Label {
                                                text: "端口号"
                                                font.pixelSize: 12
                                                color: "#344054"
                                            }
                                            SpinBox {
                                                id: tunePortField
                                                Layout.fillWidth: true
                                                from: 1
                                                to: 65535
                                                value: appState.connectionPort
                                                editable: true
                                                implicitHeight: 30
                                            }

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 10
                                                AppButton {
                                                    text: "建立连接"
                                                    implicitWidth: 84
                                                    implicitHeight: 28
                                                    onClicked: appState.connectToDevice(tuneHostField.text, tunePortField.value)
                                                }
                                                AppButton {
                                                    text: "断开连接"
                                                    implicitWidth: 84
                                                    implicitHeight: 28
                                                    onClicked: appState.disconnectFromDevice()
                                                }
                                                Item { Layout.fillWidth: true }
                                            }

                                            Label {
                                                text: "状态: " + (appState.connected ? "已连接" : "未连接")
                                                font.pixelSize: 12
                                                color: appState.connected ? "#027a48" : "#475467"
                                            }
                                            Item { Layout.fillHeight: true }
                                        }
                                    }

                                    Rectangle {
                                        color: "#f6f6f7"
                                        border.color: "#c9ccd2"
                                        radius: 2
                                        clip: true

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 6

                                            RowLayout {
                                                Layout.fillWidth: true
                                                Label {
                                                    text: "AI智能助手"
                                                    color: "#344054"
                                                    font.pixelSize: 14
                                                    font.family: root.uiFontFamily
                                                }
                                                Label {
                                                    text: "(" + (root.tuneAiModel && root.tuneAiModel.trim().length > 0 ? root.tuneAiModel : "未设置") + ")"
                                                    color: "#475467"
                                                    font.pixelSize: 12
                                                }
                                                Item { Layout.fillWidth: true }
                                                AppButton {
                                                    text: "模型配置"
                                                    implicitWidth: 74
                                                    enabled: !tuneAiBusy
                                                    onClicked: {
                                                        tuneAiLoadProviderConfig()
                                                        tuneAiProviderDialog.open()
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                color: "#f6f6f7"
                                                border.color: "#c9ccd2"
                                                radius: 2
                                                clip: true

                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                spacing: 6

                                                RowLayout {
                                                    Layout.fillWidth: true
                                                    Item { Layout.fillWidth: true }
                                                    AppButton {
                                                        text: "取消生成"
                                                        visible: tuneAiBusy
                                                        enabled: tuneAiBusy
                                                        implicitWidth: 84
                                                        onClicked: tuneAiCancel()
                                                    }
                                                }

                                                AppCheckBox {
                                                    text: "附带设备实时数据"
                                                    checked: root.tuneAiUseDeviceContext
                                                    enabled: !tuneAiBusy
                                                    onToggled: root.tuneAiUseDeviceContext = checked
                                                }

                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    Layout.minimumHeight: 140
                                                    color: "#ffffff"
                                                    radius: 2
                                                    clip: true

                                                    ListView {
                                                        id: tuneAiChatList
                                                        anchors.fill: parent
                                                        anchors.margins: 8
                                                        spacing: 8
                                                        model: tuneAiActiveMessages()
                                                        delegate: Item {
                                                            width: tuneAiChatList.width
                                                            property bool assistant: modelData.role === "assistant"
                                                            implicitHeight: (assistant ? leftRow.implicitHeight : rightRow.implicitHeight) + 4

                                                            RowLayout {
                                                                id: leftRow
                                                                anchors.left: parent.left
                                                                anchors.leftMargin: 6
                                                                anchors.top: parent.top
                                                                spacing: 2
                                                                visible: assistant

                                                                ColumnLayout {
                                                                    spacing: 0
                                                                    Layout.alignment: Qt.AlignTop
                                                                    Layout.maximumWidth: tuneAiChatList.width * 0.9
                                                                    Rectangle {
                                                                        color: "#eef6ff"
                                                                        radius: 9
                                                                        implicitHeight: leftMsg.implicitHeight + 14
                                                                        implicitWidth: Math.min(tuneAiChatList.width * 0.9, leftMsg.implicitWidth + 16)
                                                                        Text {
                                                                            id: leftMsg
                                                                            anchors.left: parent.left
                                                                            anchors.leftMargin: 8
                                                                            anchors.right: undefined
                                                                            anchors.verticalCenter: parent.verticalCenter
                                                                            wrapMode: Text.Wrap
                                                                            font.pixelSize: 12
                                                                            color: "#1f2937"
                                                                            width: Math.min(tuneAiChatList.width * 0.9 - 16, implicitWidth)
                                                                            text: modelData.content ? modelData.content : ""
                                                                        }
                                                                    }
                                                                }
                                                            }

                                                            RowLayout {
                                                                id: rightRow
                                                                anchors.right: parent.right
                                                                anchors.rightMargin: 6
                                                                anchors.top: parent.top
                                                                spacing: 2
                                                                visible: !assistant

                                                                ColumnLayout {
                                                                    spacing: 0
                                                                    Layout.alignment: Qt.AlignTop
                                                                    Layout.maximumWidth: tuneAiChatList.width * 0.9
                                                                    Rectangle {
                                                                        color: "#f3f4f6"
                                                                        radius: 9
                                                                        implicitHeight: rightMsg.implicitHeight + 14
                                                                        implicitWidth: Math.min(tuneAiChatList.width * 0.9, rightMsg.implicitWidth + 16)
                                                                        Text {
                                                                            id: rightMsg
                                                                            anchors.right: parent.right
                                                                            anchors.rightMargin: 8
                                                                            anchors.left: undefined
                                                                            anchors.verticalCenter: parent.verticalCenter
                                                                            wrapMode: Text.Wrap
                                                                            horizontalAlignment: Text.AlignRight
                                                                            font.pixelSize: 12
                                                                            color: "#111827"
                                                                            width: Math.min(tuneAiChatList.width * 0.9 - 16, implicitWidth)
                                                                            text: modelData.content ? modelData.content : ""
                                                                        }
                                                                    }
                                                                }

                                                            }
                                                        }
                                                        onCountChanged: if (count > 0) positionViewAtEnd()
                                                    }
                                                }

                                                RowLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 6
                                                    TextField {
                                                        id: tuneAiChatInputField
                                                        Layout.fillWidth: true
                                                        implicitHeight: 30
                                                        text: root.tuneAiChatInput
                                                        placeholderText: "输入问题，支持连续追问"
                                                        enabled: !tuneAiBusy
                                                        onTextEdited: root.tuneAiChatInput = text
                                                        onAccepted: tuneAiSendChat()
                                                    }
                                                    AppButton {
                                                        text: "发送"
                                                        implicitWidth: 58
                                                        enabled: !tuneAiBusy
                                                        onClicked: tuneAiSendChat()
                                                    }
                                                }

                                                Label {
                                                    Layout.fillWidth: true
                                                    visible: tuneAiBusy && !tuneAiSummaryDialog.visible
                                                    text: tuneAiBusyText()
                                                    color: "#2563eb"
                                                    font.pixelSize: 12
                                                }
                                            }
                                        }
                                    }
                                        }
                                }
                            }
                        }
                    }
                }

                Item {
                    id: monitorPage
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 31
                            color: ThemeLib.Theme.color.toolbarBg
                            border.color: ThemeLib.Theme.color.borderStrong

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 3
                                spacing: 4
                                AppButton { text: "打开方法"; implicitWidth: 66 }
                                AppButton { text: "运行方法"; implicitWidth: 66; onClicked: appState.startScan(10, 110) }
                                AppButton { text: "暂停刷新"; implicitWidth: 66 }
                                AppButton { text: "数据另存"; implicitWidth: 66 }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: appState.connected ? (appState.scanning ? "扫描运行中" : "设备空闲") : "设备未连接"
                                    color: ThemeLib.Theme.color.textSecondary
                                    font.pixelSize: 12
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: ThemeLib.Theme.color.borderDefault

                                Loader {
                                    anchors.fill: parent
                                    active: !root.isWasm
                                    sourceComponent: monitorChartsComponent
                                }

                                Component {
                                    id: monitorChartsComponent
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        spacing: 4

                                        ChartView {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 1.2
                                            antialiasing: true
                                            legend.visible: false
                                            title: "RIC"
                                            backgroundColor: ThemeLib.Theme.color.chartBg
                                            plotAreaColor: ThemeLib.Theme.color.chartBg
                                            ValueAxis { id: monitorRx; min: 0; max: 240 }
                                            ValueAxis { id: monitorRy; min: 0; max: 2000 }
                                            LineSeries {
                                                id: monitorRicSeries
                                                axisX: monitorRx
                                                axisY: monitorRy
                                                color: ThemeLib.Theme.color.linePrimary
                                                width: 2
                                            }
                                            Component.onCompleted: appState.bindRicSeries(monitorRicSeries)
                                        }

                                        ChartView {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 1.2
                                            antialiasing: true
                                            legend.visible: false
                                            title: "TIC"
                                            backgroundColor: ThemeLib.Theme.color.chartBg
                                            plotAreaColor: ThemeLib.Theme.color.chartBg
                                            ValueAxis { id: monitorTx; min: 0; max: 240 }
                                            ValueAxis { id: monitorTy; min: 0; max: 250000 }
                                            LineSeries {
                                                id: monitorTicSeries
                                                axisX: monitorTx
                                                axisY: monitorTy
                                                color: ThemeLib.Theme.color.linePrimary
                                                width: 2
                                            }
                                            Component.onCompleted: appState.bindTicSeries(monitorTicSeries)
                                        }
                                    }
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    visible: root.isWasm
                                    color: "#f4f6f9"
                                    border.color: "#cfd5de"
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 6
                                        Loader {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 1.2
                                            sourceComponent: webLineChartComponent
                                            onLoaded: {
                                                item.chartTitle = "RIC"
                                                item.chartKey = "ric"
                                                item.autoX = true
                                                item.autoY = true
                                            }
                                        }
                                        Loader {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 1.2
                                            sourceComponent: webLineChartComponent
                                            onLoaded: {
                                                item.chartTitle = "TIC"
                                                item.chartKey = "tic"
                                                item.autoX = true
                                                item.autoY = true
                                            }
                                        }
                                    }
                                }
                            }

                            AppPanel {
                                Layout.preferredWidth: 420
                                Layout.fillHeight: true
                                color: ThemeLib.Theme.color.panelSubtleBg

                                AppTabHeader {
                                    id: monitorTabHeader
                                    width: parent.width
                                    height: 30
                                    labels: ["参数配置", "仪器状态"]
                                    currentIndex: monitorRightStack.currentIndex
                                    onSelected: function(index) { monitorRightStack.currentIndex = index }
                                }

                                StackLayout {
                                    id: monitorRightStack
                                    anchors.top: monitorTabHeader.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 6
                                            anchors.margins: 6

                                            GroupBox {
                                                title: "检测器"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    spacing: 24
                                                    RadioButton {
                                                        text: "倍增器"
                                                        checked: root.monitorDetectorMode === 0
                                                        onToggled: if (checked) root.monitorDetectorMode = 0
                                                        font.pixelSize: 12
                                                        indicator: Rectangle {
                                                            implicitWidth: 12
                                                            implicitHeight: 12
                                                            radius: 6
                                                            border.color: "#b8c0cc"
                                                            color: "#ffffff"
                                                            Rectangle {
                                                                anchors.centerIn: parent
                                                                width: 7
                                                                height: 7
                                                                radius: 4
                                                                color: "#2f3440"
                                                                visible: parent.parent.checked
                                                            }
                                                        }
                                                    }
                                                    RadioButton {
                                                        text: "法拉第盘"
                                                        checked: root.monitorDetectorMode === 1
                                                        onToggled: if (checked) root.monitorDetectorMode = 1
                                                        font.pixelSize: 12
                                                        indicator: Rectangle {
                                                            implicitWidth: 12
                                                            implicitHeight: 12
                                                            radius: 6
                                                            border.color: "#b8c0cc"
                                                            color: "#ffffff"
                                                            Rectangle {
                                                                anchors.centerIn: parent
                                                                width: 7
                                                                height: 7
                                                                radius: 4
                                                                color: "#2f3440"
                                                                visible: parent.parent.checked
                                                            }
                                                        }
                                                    }
                                                    Item { Layout.fillWidth: true }
                                                }
                                            }

                                            GroupBox {
                                                title: "稳定区"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    spacing: 24
                                                    RadioButton {
                                                        text: "稳定区一"
                                                        checked: root.monitorStabilityMode === 0
                                                        onToggled: if (checked) root.monitorStabilityMode = 0
                                                        font.pixelSize: 12
                                                        indicator: Rectangle {
                                                            implicitWidth: 12
                                                            implicitHeight: 12
                                                            radius: 6
                                                            border.color: "#b8c0cc"
                                                            color: "#ffffff"
                                                            Rectangle {
                                                                anchors.centerIn: parent
                                                                width: 7
                                                                height: 7
                                                                radius: 4
                                                                color: "#2f3440"
                                                                visible: parent.parent.checked
                                                            }
                                                        }
                                                    }
                                                    RadioButton {
                                                        text: "稳定区三"
                                                        checked: root.monitorStabilityMode === 1
                                                        onToggled: if (checked) root.monitorStabilityMode = 1
                                                        font.pixelSize: 12
                                                        indicator: Rectangle {
                                                            implicitWidth: 12
                                                            implicitHeight: 12
                                                            radius: 6
                                                            border.color: "#b8c0cc"
                                                            color: "#ffffff"
                                                            Rectangle {
                                                                anchors.centerIn: parent
                                                                width: 7
                                                                height: 7
                                                                radius: 4
                                                                color: "#2f3440"
                                                                visible: parent.parent.checked
                                                            }
                                                        }
                                                    }
                                                    Item { Layout.fillWidth: true }
                                                }
                                            }

                                            GroupBox {
                                                title: "扫描方式"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    spacing: 24
                                                    RadioButton {
                                                        text: "Full"
                                                        checked: root.monitorScanMode === 0
                                                        onToggled: if (checked) root.monitorScanMode = 0
                                                        font.pixelSize: 12
                                                        indicator: Rectangle {
                                                            implicitWidth: 12
                                                            implicitHeight: 12
                                                            radius: 6
                                                            border.color: "#b8c0cc"
                                                            color: "#ffffff"
                                                            Rectangle {
                                                                anchors.centerIn: parent
                                                                width: 7
                                                                height: 7
                                                                radius: 4
                                                                color: "#2f3440"
                                                                visible: parent.parent.checked
                                                            }
                                                        }
                                                    }
                                                    RadioButton {
                                                        text: "SIM"
                                                        checked: root.monitorScanMode === 1
                                                        onToggled: if (checked) root.monitorScanMode = 1
                                                        font.pixelSize: 12
                                                        indicator: Rectangle {
                                                            implicitWidth: 12
                                                            implicitHeight: 12
                                                            radius: 6
                                                            border.color: "#b8c0cc"
                                                            color: "#ffffff"
                                                            Rectangle {
                                                                anchors.centerIn: parent
                                                                width: 7
                                                                height: 7
                                                                radius: 4
                                                                color: "#2f3440"
                                                                visible: parent.parent.checked
                                                            }
                                                        }
                                                    }
                                                    Item { Layout.fillWidth: true }
                                                }
                                            }

                                            GroupBox {
                                                title: "全扫描设置"
                                                Layout.fillWidth: true
                                                visible: root.monitorScanMode === 0
                                                GridLayout {
                                                    columns: 2
                                                    rowSpacing: 8
                                                    columnSpacing: 8
                                                    Label { text: "起始质量数:"; Layout.preferredWidth: 96 }
                                                    TextField { text: "0"; implicitWidth: 120; implicitHeight: 32 }
                                                    Label { text: "终止质量数:"; Layout.preferredWidth: 96 }
                                                    TextField { text: "200"; implicitWidth: 120; implicitHeight: 32 }
                                                }
                                            }

                                            GroupBox {
                                                title: "SIM 参数表"
                                                Layout.fillWidth: true
                                                visible: root.monitorScanMode === 1

                                                ColumnLayout {
                                                    width: parent.width
                                                    spacing: 0

                                                    RowLayout {
                                                        Layout.fillWidth: true
                                                        spacing: 0
                                                        Rectangle {
                                                            Layout.preferredWidth: 52
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "选择"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 92
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "质量数"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 96
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "质量宽度"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 96
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "时间ms"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Item { Layout.fillWidth: true }
                                                    }

                                                    Repeater {
                                                        model: monitorSimTableModel
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            spacing: 0

                                                            Rectangle {
                                                                Layout.preferredWidth: 52
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                AppCheckBox {
                                                                    anchors.centerIn: parent
                                                                    checked: model.enabled
                                                                    text: ""
                                                                    onToggled: monitorSimTableModel.setProperty(index, "enabled", checked)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 92
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.mass
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: monitorSimTableModel.setProperty(index, "mass", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 96
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.width
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: monitorSimTableModel.setProperty(index, "width", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 96
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.time
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: monitorSimTableModel.setProperty(index, "time", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }
                                                            Item { Layout.fillWidth: true }
                                                        }
                                                    }
                                                }
                                            }

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 6
                                                AppButton {
                                                    text: "设置"
                                                    implicitWidth: 72
                                                    onClicked: appState.toastRequested("SIM 参数已更新")
                                                }
                                                Item { Layout.fillWidth: true }
                                            }
                                        }
                                    }

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 10
                                            anchors.margins: 8

                                            GroupBox {
                                                title: "部件温度[℃]"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 10
                                                    spacing: 8
                                                    Repeater {
                                                        model: ["进样管路1", "进样管路2", "进样腔体", "真空腔体"]
                                                        delegate: ColumnLayout {
                                                            spacing: 4
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "0"
                                                                font.pixelSize: 13
                                                                font.bold: true
                                                                color: "#344054"
                                                            }
                                                            Rectangle {
                                                                Layout.alignment: Qt.AlignHCenter
                                                                width: 20
                                                                height: 156
                                                                radius: 3
                                                                border.color: "#c8cdd6"
                                                                color: "#f7f8fa"
                                                                Rectangle {
                                                                    anchors.left: parent.left
                                                                    anchors.right: parent.right
                                                                    anchors.bottom: parent.bottom
                                                                    height: appState.connected ? 36 : 0
                                                                    radius: 2
                                                                    color: "#57a5ff"
                                                                }
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "100"
                                                                font.pixelSize: 12
                                                                color: "#98a2b3"
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: modelData
                                                                wrapMode: Text.WordWrap
                                                                font.pixelSize: 12
                                                                color: "#475467"
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "部件状态"
                                                Layout.fillWidth: true

                                                ColumnLayout {
                                                    width: parent.width
                                                    spacing: 0
                                                    property var names: [
                                                        "+24V", "灯丝电流[A]", "电子能量[eV]", "发射电流[μA]", "射频扫描电压[V]", "射频电流[A]",
                                                        "倍增器电压[V]", "前预四极[V]", "打拿极[V]", "推斥极[V]", "透镜1[V]", "透镜2[V]"
                                                    ]
                                                    property var values: [
                                                        appState.connected ? "24" : "0",
                                                        "0.0", "0.0", "0.0",
                                                        "100.0", "20.0",
                                                        "1200.0", "0.0", "0.0", "0.0", "0.0", "0.0"
                                                    ]

                                                    Repeater {
                                                        model: 12
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            Layout.preferredHeight: 28
                                                            spacing: 0
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.45
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.left: parent.left
                                                                    anchors.leftMargin: 8
                                                                    anchors.verticalCenter: parent.verticalCenter
                                                                    text: parent.parent.parent.names[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.0
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.centerIn: parent
                                                                    text: parent.parent.parent.values[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            color: "#e9edf3"
                            border.color: ThemeLib.Theme.color.borderStrong

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 6
                                anchors.rightMargin: 6
                                AppButton {
                                    text: appState.scanning ? "停止扫描" : "开始扫描"
                                    implicitWidth: 72
                                    onClicked: appState.scanning ? appState.stopScan() : appState.startScan(10, 110)
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: "真空度: " + f2(appState.vacuumPa) + " Pa"
                                    font.pixelSize: 12
                                    color: ThemeLib.Theme.color.textSecondary
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Rectangle {
                        anchors.fill: parent
                        color: "#ffffff"
                        border.color: ThemeLib.Theme.color.borderDefault
                        Text {
                            anchors.centerIn: parent
                            text: "方法页开发中"
                            color: ThemeLib.Theme.color.textSecondary
                            font.pixelSize: 14
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            color: "#c8c3e8"
                            border.color: "#b4aed9"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                anchors.topMargin: 4
                                anchors.bottomMargin: 4

                                AppButton {
                                    text: "打开数据"
                                    implicitWidth: 96
                                    implicitHeight: 28
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.topMargin: 4
                            spacing: 0

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#f7f7f7"
                                border.color: "#a9adb7"

                                Loader {
                                    anchors.fill: parent
                                    active: !root.isWasm
                                    sourceComponent: dataChartsComponent
                                }

                                Component {
                                    id: dataChartsComponent
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        spacing: 2

                                        ChartView {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 5
                                            antialiasing: true
                                            legend.visible: false
                                            title: "全扫描"
                                            backgroundColor: "#f4f5f7"
                                            plotAreaColor: "#f4f5f7"
                                            ValueAxis { id: dataSx; min: 0; max: 620 }
                                            ValueAxis { id: dataSy; min: 0; max: 1000 }
                                            LineSeries {
                                                axisX: dataSx
                                                axisY: dataSy
                                                color: ThemeLib.Theme.color.linePrimary
                                                width: 2
                                                XYPoint { x: 0; y: 0 }
                                                XYPoint { x: 620; y: 0 }
                                            }
                                        }

                                        ChartView {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 2
                                            antialiasing: true
                                            legend.visible: false
                                            title: "TIC"
                                            backgroundColor: "#f4f5f7"
                                            plotAreaColor: "#f4f5f7"
                                            ValueAxis { id: dataTx; min: 0; max: 1000 }
                                            ValueAxis { id: dataTy; min: 0; max: 1000 }
                                            LineSeries {
                                                axisX: dataTx
                                                axisY: dataTy
                                                color: ThemeLib.Theme.color.linePrimary
                                                width: 2
                                                XYPoint { x: 0; y: 0 }
                                                XYPoint { x: 1000; y: 0 }
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    visible: root.isWasm
                                    color: "#f4f6f9"
                                    border.color: "#cfd5de"
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 6
                                        Loader {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 5
                                            sourceComponent: webLineChartComponent
                                            onLoaded: {
                                                item.chartTitle = "全扫描"
                                                item.chartKey = "spectrum"
                                                item.autoX = false
                                                item.autoY = true
                                                item.fixedXMin = 0
                                                item.fixedXMax = 620
                                            }
                                        }
                                        Loader {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredHeight: 2
                                            sourceComponent: webLineChartComponent
                                            onLoaded: {
                                                item.chartTitle = "TIC"
                                                item.chartKey = "tic"
                                                item.autoX = true
                                                item.autoY = true
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.preferredWidth: 258
                                Layout.minimumWidth: 258
                                Layout.maximumWidth: 258
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: "#a9adb7"

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    anchors.bottomMargin: 8
                                    spacing: 0
                                    Label {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 30
                                        text: "配置信息"
                                        font.pixelSize: 12
                                        font.bold: true
                                        color: "#4a4a4a"
                                        verticalAlignment: Text.AlignVCenter
                                        padding: 2
                                    }
                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 1
                                        color: "#e4e7ec"
                                    }
                                    Item { Layout.fillHeight: true }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 24
                            color: "#f6f3d7"
                            border.color: "#c7c19a"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 4
                                Label {
                                    text: "运行状态"
                                    color: "#4a4a4a"
                                    font.pixelSize: 12
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }

                Item {
                    id: settingsPage
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 0
                            Layout.margins: 6

                            ColumnLayout {
                                Layout.preferredWidth: 290
                                Layout.maximumWidth: 290
                                Layout.fillHeight: true
                                spacing: 0

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 29
                                    color: "transparent"

                                    RowLayout {
                                        anchors.fill: parent
                                        spacing: 0
                                        Repeater {
                                            model: ["本地连接", "远程中转服务"]
                                            delegate: Rectangle {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 28
                                                color: index === root.settingsConnectionTabIndex ? "#ffffff" : "#e7e7e7"
                                                border.color: "#bcbcbc"
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData
                                                    font.pixelSize: 12
                                                    font.bold: index === root.settingsConnectionTabIndex
                                                    color: "#333333"
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        if (root.settingsConnectionTabIndex === index) {
                                                            return
                                                        }
                                                        if (root.settingsConnectionTabIndex === 0) {
                                                            root.settingsLocalHost = settingHostField.text
                                                            root.settingsLocalPort = Number(settingPortField.text)
                                                        } else {
                                                            root.settingsRemoteHost = settingHostField.text
                                                            root.settingsRemotePort = Number(settingPortField.text)
                                                        }
                                                        root.settingsConnectionTabIndex = index
                                                        settingHostField.text = index === 0 ? root.settingsLocalHost : root.settingsRemoteHost
                                                        settingPortField.text = String(index === 0 ? root.settingsLocalPort : root.settingsRemotePort)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 152
                                    color: "#ffffff"
                                    border.color: "#bcbcbc"
                                    clip: true

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Label {
                                                text: root.settingsConnectionTabIndex === 0 ? "本地连接" : "远程中转服务"
                                                font.pixelSize: 12
                                                font.bold: true
                                                color: "#222222"
                                            }
                                            Item { Layout.fillWidth: true }
                                            Label {
                                                text: appState.connected
                                                    ? ("已连接  " + settingHostField.text + ":" + settingPortField.text)
                                                    : "未连接"
                                                font.pixelSize: 12
                                                font.bold: true
                                                color: appState.connected ? "#027a48" : "#6b7280"
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                                horizontalAlignment: Text.AlignRight
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "IP地址"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingHostField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: root.settingsConnectionTabIndex === 0 ? root.settingsLocalHost : root.settingsRemoteHost
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "端口号"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingPortField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                inputMethodHints: Qt.ImhDigitsOnly
                                                text: String(root.settingsConnectionTabIndex === 0 ? root.settingsLocalPort : root.settingsRemotePort)
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 10
                                            Item { Layout.fillWidth: true }
                                            AppButton {
                                                text: "建立连接"
                                                implicitWidth: 92
                                                implicitHeight: 26
                                                onClicked: {
                                                    const port = Number(settingPortField.text)
                                                    if (root.settingsConnectionTabIndex === 0) {
                                                        root.settingsLocalHost = settingHostField.text
                                                        root.settingsLocalPort = port
                                                    } else {
                                                        root.settingsRemoteHost = settingHostField.text
                                                        root.settingsRemotePort = port
                                                    }
                                                    root.settingsRunStatus = "运行状态    正在建立连接"
                                                    appState.connectToDevice(settingHostField.text, port)
                                                }
                                            }
                                            AppButton {
                                                text: "断开连接"
                                                implicitWidth: 92
                                                implicitHeight: 26
                                                enabled: appState.connected
                                                onClicked: {
                                                    root.settingsRunStatus = "运行状态    未连接"
                                                    appState.disconnectFromDevice()
                                                }
                                            }
                                            Item { Layout.fillWidth: true }
                                        }
                                    }
                                }

                                Item { Layout.preferredHeight: 10 }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 128
                                    color: "#ffffff"
                                    border.color: "#bcbcbc"
                                    clip: true

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 6
                                        Label { text: "存储"; font.pixelSize: 12; font.bold: true; color: "#222222" }
                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "文件路径"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingPathField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: "/tmp/deviceapp/device-app/frames"
                                            }
                                            AppButton { text: "..."; implicitWidth: 34; implicitHeight: 28 }
                                        }
                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "文件名前缀"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingPrefixField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: "Test"
                                            }
                                            AppButton {
                                                text: "设置"
                                                implicitWidth: 60
                                                implicitHeight: 28
                                                onClicked: root.settingsRunStatus = "运行状态    存储设置已保存"
                                            }
                                        }
                                        AppCheckBox { text: "自动保存"; checked: root.settingsAutoSave; onToggled: root.settingsAutoSave = checked }
                                    }
                                }
                                Item { Layout.fillHeight: true }
                            }

                            Rectangle {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                                color: "#bcbcbc"
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: "#bcbcbc"
                                clip: true
                                Layout.leftMargin: 10

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 6
                                    Label { text: "管道服务测试"; font.pixelSize: 12; font.bold: true; color: "#222222" }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 28
                                        spacing: 5
                                        TextField {
                                            id: payloadInput
                                            Layout.fillWidth: true
                                            implicitHeight: 26
                                            font.pixelSize: 13
                                            topPadding: 3
                                            bottomPadding: 3
                                            text: "&&$$:11:2,1;@@"
                                        }
                                        AppButton {
                                            text: "发送数据"
                                            implicitWidth: 76
                                            implicitHeight: 26
                                            onClicked: {
                                                if (showSendCheck.checked) {
                                                    serviceLog.text += "\\n[发送] " + payloadInput.text
                                                }
                                            }
                                        }
                                        AppCheckBox { id: showSendCheck; text: "显示发送"; checked: true }
                                        AppCheckBox { id: showReceiveCheck; text: "显示接收"; checked: true }
                                        AppButton { text: "清空数据"; implicitWidth: 76; implicitHeight: 26; onClicked: serviceLog.text = "" }
                                    }

                                    TextArea {
                                        id: serviceLog
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        readOnly: true
                                        Layout.topMargin: 2
                                        text: ""
                                        placeholderText: "暂无日志。发送管道数据后，日志会显示在这里。"
                                        background: Rectangle {
                                            color: "#ffffff"
                                            border.color: "#a9a9a9"
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 24
                            color: "#f6f3d7"
                            border.color: "#c7c19a"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 4
                                anchors.rightMargin: 4
                                Label {
                                    text: root.settingsRunStatus
                                    color: "#222222"
                                    font.pixelSize: 12
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }
            }
        }
    }
}
