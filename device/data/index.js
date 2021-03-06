const PORT_COUNT = 4;
const DEFAULT_RULES = [...Array(PORT_COUNT)].map((u, i) => []);
const CONDITIONS = {
	"time": {
		name: "The time of day is",
		icon: "⏰",
		min: 0,
		max: 1439,
		type: "time"
	},
	"dayOfWeek": {
		name: "The day of week is",
		icon: "📅",
		values: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"],
		none: "(none)"
	},
	"temperature": {
		name: "The temperature is",
		icon: "🌡️",
		min: -10,
		max: 60,
		unit: "°C"
	},
	"humidity": {
		name: "The humidity is",
		icon: "💧",
		min: 0,
		max: 100,
		unit: "%"
	},
	"light": {
		name: "The light level is",
		icon: "💡",
		min: 0,
		max: 100,
		unit: "%"
	},
	"proximity": {
		name: "Motion detector",
		icon: "🍃",
		values: ["Motion detected"],
		none: "No motion detected"
	}
};
const COMPARISONS = ["less than", "greater than", "between", "not between"];
const COMPARISONS_TIME = ["before", "after", "between", "not between"];

function setup() {
	"use strict";

	class MainScreen extends React.Component {

		constructor(props) {
			super(props);
			this.editOrSave = this.editOrSave.bind(this);
			this.state = {
				edit: false,
				saveDisabled: false,
				message: "",
				rules: Object.assign([], "rules" in STORED_SETTINGS ? STORED_SETTINGS["rules"] : DEFAULT_RULES)
			};
		}

		editOrSave(event) {
			if (this.state.edit) {
				this.setState({saveDisabled: true});
				const rules = this.state.rules;
				rules.map(port => port.map(rule => Object.keys(rule).map(conditionId => {
					const details = CONDITIONS[conditionId];
					if (!("values" in details)) {
						const detailsMin = details["min"];
						const detailsMax = details["max"];
						const condition = rule[conditionId];
						if (Number.isNaN(condition["min"]) || condition["min"] === "") {
							condition["min"] = detailsMin;
						}
						if (Number.isNaN(condition["max"]) || condition["max"] === "") {
							condition["max"] = detailsMax;
						}
						condition["min"] = Math.min(Math.max(detailsMin, condition["min"]), detailsMax);
						condition["max"] = Math.min(Math.max(detailsMin, condition["max"]), detailsMax);
						if (condition["comparison"] >= 2 && condition["min"] > condition["max"]) {
							const temp = condition["min"];
							condition["min"] = condition["max"];
							condition["max"] = temp;
						}
					}
				})));
				fetch("/settings", {
					method: "POST",
					body: JSON.stringify({timezone: (new Date()).getTimezoneOffset() * -60, rules: rules}),
					headers: {
						"Content-type": "application/json; charset=UTF-8"
					}
				}).then(response => response.json()).then(data => {
					this.setState({
						edit: false,
						saveDisabled: false,
						message: "Configuration saved. You're good to go!"
					});
					setTimeout(() => this.setState({message: ""}), 5000);
				}).catch(error => {
					this.setState({
						edit: true,
						saveDisabled: false,
						message: "Failed to save. Please make sure that the device is connected to the network and try again."
					});
					setTimeout(() => this.setState({message: ""}), 5000);
				});
			} else {
				this.setState({edit: true});
			}
		}

		render() {
			return (
				<div>
					<h1>Smart Home Device Configuration</h1>
					<table>
						<tbody>
						{[...Array(PORT_COUNT)].map((u, i) =>
							<Port key={i} rules={this.state.rules[i]} index={i} edit={this.state.edit}/>
						)}
						</tbody>
					</table>
					<br/>
					<input
						className="input_button"
						type="submit"
						disabled={this.state.saveDisabled}
						value={this.state.edit ? (this.state.saveDisabled ? "Please Wait" : "Save") : "Edit"}
						onClick={this.editOrSave}
					/>
					<p>&nbsp;{this.state.message}&nbsp;</p>
				</div>
			);
		}
	}

	class Port extends React.Component {

		constructor(props) {
			super(props);
			this.addRule = this.addRule.bind(this);
			this.addCondition = this.addCondition.bind(this);
			this.deleteCondition = this.deleteCondition.bind(this);
		}

		addRule(event) {
			this.props.rules.push({});
			this.addCondition(this.props.rules.length - 1, Object.keys(CONDITIONS)[0], event);
			this.setState({});
		}

		addCondition(ruleIndex, conditionId, event) {
			const details = CONDITIONS[conditionId];
			const isDiscrete = "values" in details;
			if (isDiscrete) {
				this.props.rules[ruleIndex][conditionId] = {values: []};
			} else {
				this.props.rules[ruleIndex][conditionId] = {comparison: 0, min: details["min"], max: details["max"]};
			}
			this.setState({});
		}

		deleteCondition(ruleIndex, conditionId, event) {
			delete this.props.rules[ruleIndex][conditionId];
			if (Object.keys(this.props.rules[ruleIndex]).length === 0) {
				this.props.rules.splice(ruleIndex, 1);
			}
			this.setState({});
		}

		portName(index) {
			const {rules, edit} = this.props;
			const ruleCount = rules.length;
			const canAddRule = edit && (ruleCount === 0 || Object.keys(this.props.rules[ruleCount - 1]).length > 0);
			return (
				<td className="column_fixed_width column_port_name" rowSpan={Math.max(rules.length, 1)}>
					<h2>Port {index + 1}</h2>
					{canAddRule ? <a onClick={this.addRule}>Add Rule</a> : null}
				</td>
			);
		}

		render() {
			const {rules, index, edit} = this.props;
			const ruleCount = rules.length;
			if (ruleCount === 0) {
				return (
					<tr>
						{this.portName(index)}
						<td className="column_fixed_width">Always off</td>
						<td className="column_conditions"/>
						<td className="column_add_conditions"/>
					</tr>
				);
			} else {
				return (
					<>
						{[...Array(ruleCount)].map((u, ruleIndex) => {
							const availableConditions = Object.keys(CONDITIONS).filter(key => !(key in this.props.rules[ruleIndex]));
							return (
								<tr key={ruleIndex}>
									{ruleIndex === 0 ? this.portName(index) : null}
									<td className="column_fixed_width">{ruleIndex === 0 ? "Turns on when" : "or when"}</td>
									<td className="column_conditions">
										{Object.keys(rules[ruleIndex]).map(conditionId =>
											<Condition
												conditionId={conditionId}
												key={`condition_${ruleIndex}_${conditionId}`}
												condition={rules[ruleIndex][conditionId]}
												onDelete={() => this.deleteCondition(ruleIndex, conditionId)}
												edit={edit}
											/>
										)}
									</td>
									<td className="column_add_conditions">
										{edit ? availableConditions.map(conditionId =>
											<AddButton
												key={`add_${ruleIndex}_${conditionId}`}
												icon={CONDITIONS[conditionId]["icon"]}
												onClick={(event) => this.addCondition(ruleIndex, conditionId, event)}
											/>
										) : null}
									</td>
								</tr>
							);
						})}
					</>
				);
			}
		}
	}

	function AddButton(props) {
		return (
			<div className="clickable" onClick={props["onClick"]}>
				<div className="add_button text">+</div>
				<div className="add_button">{props["icon"]}</div>
			</div>
		);
	}

	class Condition extends React.Component {

		constructor(props) {
			super(props);
			this.selectComparison = this.selectComparison.bind(this);
			this.changeValue = this.changeValue.bind(this);
			this.changeDiscrete = this.changeDiscrete.bind(this);
		}

		selectComparison(event) {
			const {condition} = this.props;
			condition["comparison"] = parseInt(event.target.value);
			this.setState({});
		}

		changeValue(dataType, event) {
			const {conditionId, condition} = this.props;
			const details = CONDITIONS[conditionId];
			const value = event.target.value;
			if (details["type"] === "time") {
				const timeSplit = value.split(":");
				condition[dataType] = parseInt(timeSplit[0]) * 60 + parseInt(timeSplit[1]);
			} else {
				condition[dataType] = parseInt(value);
			}
		}

		changeDiscrete(event) {
			const {condition} = this.props;
			const value = parseInt(event.target.value);
			if (event.target.checked) {
				if (!condition["values"].includes(value)) {
					condition["values"].push(value);
					condition["values"].sort();
				}
			} else {
				condition["values"].splice(condition["values"].indexOf(value), 1);
			}
		}

		displayValue(value) {
			const {conditionId} = this.props;
			if (CONDITIONS[conditionId]["type"] === "time") {
				const hour = parseInt(value.toString().substring(0, 2));
				const pm = hour >= 12;
				let hourReadable = hour % 12;
				hourReadable = hourReadable === 0 ? 12 : hourReadable;
				return hourReadable.toString() + value.toString().substring(2) + " " + (pm ? "PM" : "AM");
			} else {
				return value;
			}
		}

		render() {
			const {conditionId, condition, onDelete, edit} = this.props;
			const details = CONDITIONS[conditionId];
			const isDiscrete = "values" in details;
			const comparison = condition["comparison"];
			const isTime = details["type"] === "time";
			return (
				<div className={`condition ${conditionId} ${edit ? "" : "small"}`}>
					{edit ? <div className="clickable delete_button" onClick={onDelete}>✖</div> : null}
					{details["icon"]} {details["name"]}
					<br/>
					<br/>
					{isDiscrete ?
						<div>
							{edit ? [...Array(details["values"].length)].map((u, index) => {
								const value = details["values"][index];
								return (
									<div key={index}>
										<label>
											<input
												className="input_checkbox"
												defaultChecked={condition["values"].includes(index)}
												type="checkbox"
												value={index}
												onChange={this.changeDiscrete}
											/>
											{value}
										</label>
										<br/>
									</div>
								);
							}) : condition["values"].length > 0 ? condition["values"].map(index => details["values"][index]).toString().replace(/,/g, ", ") : details["none"]}
						</div> :
						edit ? <div>
								<select
									defaultValue={comparison}
									className="input_text smaller"
									onChange={this.selectComparison}
								>
									{[...Array(COMPARISONS.length)].map((u, index) =>
										<option key={`select_option_${index}`} value={index}>
											{isTime ? COMPARISONS_TIME[index] : COMPARISONS[index]}
										</option>
									)}
								</select>
								<br/>
								<br/>
								<div hidden={comparison === 0}>
									<NumberInput
										isTime={isTime}
										placeholder={details["min"]}
										defaultValue={condition["min"]}
										onChange={(event) => this.changeValue("min", event)}
									/>
									{details["unit"]}
								</div>
								<div hidden={comparison <= 1}>
									and
									<br/>
								</div>
								<div hidden={comparison === 1}>
									<NumberInput
										isTime={isTime}
										placeholder={details["max"]}
										defaultValue={condition["max"]}
										onChange={(event) => this.changeValue("max", event)}
									/>
									{details["unit"]}
								</div>
							</div> :
							<div>
								{isTime ? COMPARISONS_TIME[comparison] : COMPARISONS[comparison]}
								&nbsp;
								{comparison === 0 ? "" : <>{isTime ? formatTime(condition["min"], true) : condition["min"]}{details["unit"]}</>}
								{comparison <= 1 ? "" : <><br/>and&nbsp;</>}
								{comparison === 1 ? "" : <>{isTime ? formatTime(condition["max"], true) : condition["max"]}{details["unit"]}</>}
							</div>
					}
				</div>
			);
		}
	}

	function NumberInput(props) {
		const {isTime, placeholder, defaultValue, onChange} = props;

		function formatNumber(event) {
			let value = event.target.value;
			if (!isTime) {
				value = value.replace(/[^0-9-]/g, "");
				if (value.length > 0 && value !== "-") {
					value = parseInt(value);
				}
				event.target.value = value;
			}
			onChange(event);
		}

		return (
			<input
				className="input_text smaller"
				type={isTime ? "time" : "text"}
				placeholder={isTime ? formatTime(placeholder) : placeholder}
				defaultValue={isTime ? formatTime(defaultValue) : defaultValue}
				onChange={(event) => formatNumber(event)}
			/>
		);
	}

	if (window.location.pathname !== "/") {
		window.location.pathname = "/";
	}
	ReactDOM.render(<MainScreen/>, document.querySelector("#react-root"));
}

function formatTime(value, readable = false) {
	const hour = Math.floor(value / 60);
	const min = Math.floor(value % 60);
	if (readable) {
		let hour12 = hour % 12;
		if (hour12 === 0) {
			hour12 = 12;
		}
		return hour12.toString() + ":" + min.toString().padStart(2, "0") + " " + (hour >= 12 ? "PM" : "AM");
	} else {
		return hour.toString().padStart(2, "0") + ":" + min.toString().padStart(2, "0");
	}
}

setup();